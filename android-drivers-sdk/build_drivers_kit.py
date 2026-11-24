#!/usr/bin/env python3
# daas © 2019–2026 Sebyone Srl
#
# File: build_drivers_kit.py — daasiot-android, android-drivers-sdk
#
# Builds the drivers_kit of every entry of android-drivers-sdk/libdaas-drivers.lock.json into
# android_daas/drivers_kit/<target>/static/: the specialized drivers of the android-drivers-sdk, compiled with CMake
# and the toolchain of the Android NDK for the ABI of the entry against only the headers of its libdaas package, with
# the precompiled blew of the DaaS Build Console (checked against the SHA-256 of the lock), and DRIVERS.txt:
#
#   libdaas_drv_ble.a, libblew.a   linked by android_daas/libdaas.cmake before libdaas_nodrivers (blew whole: its JNI
#                                  entry points are reached only from Java)
#   java/                          the Java side of blew (it.sebyone.blew), compiled into the AAR with android_daas
#
#   python3 android-drivers-sdk/build_drivers_kit.py                       every entry of the drivers lock
#   python3 android-drivers-sdk/build_drivers_kit.py --only android-arm64-v8a/static
#   python3 android-drivers-sdk/build_drivers_kit.py --blew-from <dir>      blew tarballs already downloaded
#   python3 android-drivers-sdk/build_drivers_kit.py --update-lock          write the SHA-256 of the blew downloaded
#   python3 android-drivers-sdk/build_drivers_kit.py --check                only check the installed kits (exit 1)
#   python3 android-drivers-sdk/build_drivers_kit.py --cmake-arg=-DX=Y      (repeatable)
#
# The NDK is ANDROID_NDK_HOME (or ANDROID_NDK_ROOT, --ndk). The build server is the "server" of the blew section of
# the lock, or DAAS_BUILDER_URL. Intermediate files in out/. Python 3.7 or later.
#
# Contributors:
#   Lorenzo Grillo
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-24  Lorenzo Grillo  creation: drivers_kit per lock entry (BLE driver + precompiled blew), from
#                               build_drivers_kit.py of daasiot-macos
"""Build the drivers_kit of android_daas from android-drivers-sdk/libdaas-drivers.lock.json."""

import argparse
import hashlib
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import urllib.request

SDK = os.path.abspath(os.path.dirname(__file__))
ROOT = os.path.dirname(SDK)
DLOCK = os.path.join(SDK, "libdaas-drivers.lock.json")
LLOCK = os.path.join(ROOT, "android_daas", "libdaas.lock.json")
KIT = os.path.join(ROOT, "android_daas", "drivers_kit")
OUT = os.path.join(ROOT, "out")
KIT_FILES = ["libdaas_drv_ble.a", "libblew.a", "DRIVERS.txt", "java/it/sebyone/blew/Blew.java"]
API_LEVEL = "30"          # minimum Android API of the libdaas packages


def say(msg):
    print("[drivers_kit] " + msg, flush=True)


def fail(msg):
    sys.exit("[drivers_kit] " + msg)


def load(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def driver_version(driver):
    with open(os.path.join(SDK, driver, driver + "_version.hpp"), encoding="utf-8") as fh:
        m = re.search(r'DAAS_DRIVER_VERSION_\w+\s+"([^"]+)"', fh.read())
    return m.group(1) if m else None


def write_sha(name, digest):
    """Writes the SHA-256 of one blew package into the drivers lock, keeping its layout."""
    with open(DLOCK, encoding="utf-8") as fh:
        text = fh.read()
    pat = re.compile(r'("%s"\s*:\s*\{[^}]*?"sha256"\s*:\s*")[0-9a-f]*(")' % re.escape(name))
    text, n = pat.subn(lambda m: m.group(1) + digest + m.group(2), text, count=1)
    if n != 1:
        fail("blew %s: no sha256 field in the drivers lock" % name)
    with open(DLOCK, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)


def fetch_blew(dlock, name, folder, update_lock):
    pkg = dlock["blew"]["packages"][name]
    file = os.path.basename(pkg["path"])
    if folder:
        with open(os.path.join(folder, file), "rb") as fh:
            data = fh.read()
    else:
        server = os.environ.get("DAAS_BUILDER_URL", dlock["blew"]["server"]).rstrip("/")
        url = "%s/scarica/%s" % (server, pkg["path"])
        try:
            with urllib.request.urlopen(url, timeout=120) as resp:
                data = resp.read()
        except Exception as exc:
            fail("blew %s: %s: %s (start the DaaS Build Console, set DAAS_BUILDER_URL or use --blew-from)"
                 % (name, url, exc))
    digest = hashlib.sha256(data).hexdigest()
    os.makedirs(os.path.join(OUT, "blew", "dl"), exist_ok=True)
    with open(os.path.join(OUT, "blew", "dl", file), "wb") as fh:   # for the drivers-sdk package (build_dist.py)
        fh.write(data)
    if update_lock:
        if pkg["sha256"] != digest:
            say("blew %s: SHA-256 %s written to the drivers lock (was %s)" % (file, digest, pkg["sha256"] or "empty"))
            write_sha(name, digest)
            pkg["sha256"] = digest
    elif not pkg["sha256"]:
        fail("blew %s: no SHA-256 in the drivers lock; downloaded %s: review it, then --update-lock" % (file, digest))
    elif digest != pkg["sha256"]:
        fail("blew %s: SHA-256 %s differs from the drivers lock (%s)" % (file, digest, pkg["sha256"]))
    dest = os.path.join(OUT, "blew", name)
    if os.path.isdir(dest):
        shutil.rmtree(dest)
    os.makedirs(dest)
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as tf:
        for m in tf.getmembers():
            n = m.name
            while n.startswith("./"):
                n = n[2:]
            if m.isfile() and not n.startswith("/") and ".." not in n.split("/"):
                out = os.path.join(dest, *n.split("/"))
                os.makedirs(os.path.dirname(out), exist_ok=True)
                with open(out, "wb") as fh:
                    fh.write(tf.extractfile(m).read())
    tops = os.listdir(dest)
    if not os.path.isdir(os.path.join(dest, "lib")) and len(tops) == 1 and os.path.isdir(os.path.join(dest, tops[0])):
        dest = os.path.join(dest, tops[0])          # a tarball with one top folder
    for need in (("lib", "libblew.a"), ("include", "blew", "link.h"), ("java", "it", "sebyone", "blew", "Blew.java")):
        if not os.path.isfile(os.path.join(dest, *need)):
            fail("blew %s: %s missing" % (file, "/".join(need)))
    return dest


def check(dlock):
    problems = []
    for key in dlock["kits"]:
        for f in KIT_FILES:
            if not os.path.isfile(os.path.join(KIT, *(key.split("/") + f.split("/")))):
                problems.append("missing android_daas/drivers_kit/%s/%s" % (key, f))
    return problems


def find_ndk(given):
    for ndk in (given, os.environ.get("ANDROID_NDK_HOME"), os.environ.get("ANDROID_NDK_ROOT")):
        if ndk and os.path.isfile(os.path.join(ndk, "build", "cmake", "android.toolchain.cmake")):
            return os.path.abspath(ndk)
    fail("Android NDK not found: set ANDROID_NDK_HOME (or --ndk)")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--only", action="append", metavar="KEY")
    p.add_argument("--blew-from", help="folder with the blew tarballs of the drivers lock (default: "
                   "android-drivers-sdk/blew/ when it has them, as in the drivers-sdk package; else the build server)")
    p.add_argument("--update-lock", action="store_true", help="write the SHA-256 of the blew downloaded into the lock")
    p.add_argument("--check", action="store_true")
    p.add_argument("--ndk", help="the Android NDK (default: ANDROID_NDK_HOME, ANDROID_NDK_ROOT)")
    p.add_argument("--cmake-arg", action="append", default=[], help="passed to the configure step of cmake")
    args = p.parse_args()

    dlock, llock = load(DLOCK), load(LLOCK)
    packaged = os.path.join(SDK, "blew")
    if not args.blew_from and os.path.isdir(packaged):
        args.blew_from = packaged
    if args.check:
        problems = check(dlock)
        for x in problems:
            say(x)
        if problems:
            fail("run: python3 android-drivers-sdk/build_drivers_kit.py")
        say("drivers_kit installed for " + ", ".join(dlock["kits"]))
        return
    if dlock["libdaas"] != llock["version"]:
        fail("the drivers lock is for libdaas %s, the libdaas lock is %s" % (dlock["libdaas"], llock["version"]))
    missing = [k for k in dlock["kits"] if k not in llock["packages"]]
    if missing:
        fail("not in the libdaas lock: " + ", ".join(missing))
    for key in args.only or []:
        if key not in dlock["kits"]:
            fail("%s is not in the drivers lock (%s)" % (key, ", ".join(dlock["kits"])))
    cmake = shutil.which("cmake") or fail("cmake not found")
    ndk = find_ndk(args.ndk)
    toolchain = os.path.join(ndk, "build", "cmake", "android.toolchain.cmake")

    blew_cache = {}
    for key in args.only or list(dlock["kits"]):
        kit = dlock["kits"][key]
        target, linkage = key.split("/")
        abi = target.split("-", 1)[1]
        pkg_root = os.path.join(ROOT, "android_daas", "libdaas", target, linkage)
        if not os.path.isfile(os.path.join(pkg_root, "MANIFEST.json")):
            fail("libdaas %s not installed: run tools/fetch_ld/fetch_ld.sh" % key)
        with open(os.path.join(pkg_root, "DRIVERS.txt"), encoding="utf-8") as fh:
            generic = {" ".join(l.split()[:2]).lower() for l in fh if l.strip() and not l.startswith("#")}
        for g in kit["generic"]:
            if g not in generic:
                fail("%s: generic driver %s not in the DRIVERS.txt of the package (%s)"
                     % (key, g, ", ".join(sorted(generic))))
        bname = kit["blew"]
        if bname not in blew_cache:
            blew_cache[bname] = fetch_blew(dlock, bname, args.blew_from, args.update_lock)
        blew = blew_cache[bname]
        build = os.path.join(OUT, "drivers", target)
        if os.path.isdir(build):
            shutil.rmtree(build)
        say("%s: %s with blew %s (%s)" % (key, ", ".join(kit["specialized"]), dlock["blew"]["version"], bname))
        subprocess.run([cmake, "-S", SDK, "-B", build, "-DCMAKE_TOOLCHAIN_FILE=" + toolchain, "-DANDROID_ABI=" + abi,
                        "-DANDROID_PLATFORM=android-" + API_LEVEL, "-DANDROID_STL=c++_static",
                        "-DDAAS_TARGET=" + target, "-DDAAS_ROOT=" + pkg_root, "-DBLEW_ROOT=" + blew,
                        "-DCMAKE_BUILD_TYPE=Release"] + args.cmake_arg, check=True, stdout=subprocess.DEVNULL)
        subprocess.run([cmake, "--build", build, "-j"], check=True)   # every driver and the template must build
        dest = os.path.join(KIT, target, linkage)
        if os.path.isdir(dest):
            shutil.rmtree(dest)
        os.makedirs(dest)
        lines = ["# drivers_kit of %s: specialized drivers of the android-drivers-sdk (libdaas %s)"
                 % (key, dlock["libdaas"])]
        for spec in kit["specialized"]:
            driver, version, adapter = spec.split()
            if driver_version(driver) != version:
                fail("%s: %s is %s in %s_version.hpp, the lock says %s"
                     % (key, driver, driver_version(driver), driver, version))
            if adapter != "blew":
                fail("%s: unknown adapter %s" % (key, adapter))
            shutil.copy2(os.path.join(build, "libdaas_drv_%s.a" % driver), dest)
            lines.append("%s %s libdaas_drv_%s.a (android-drivers-sdk, specialized)" % (driver.upper(), version, driver))
        shutil.copy2(os.path.join(blew, "lib", "libblew.a"), dest)
        shutil.copytree(os.path.join(blew, "java"), os.path.join(dest, "java"))
        lines.append("BLEW %s libblew.a java/it/sebyone/blew (DaaS Build Console, %s)" % (dlock["blew"]["version"], bname))
        with open(os.path.join(dest, "DRIVERS.txt"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write("\n".join(lines) + "\n")
        say("%s: android_daas/drivers_kit/%s/" % (key, key))
    if args.update_lock:
        say("drivers lock updated: review the diff of android-drivers-sdk/libdaas-drivers.lock.json")
    say("done")


if __name__ == "__main__":
    main()
