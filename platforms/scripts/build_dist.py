#!/usr/bin/env python3
# daas © 2019–2026 Sebyone Srl
#
# File: build_dist.py — daasiot-android (platforms/scripts)
#
# The packages of daasiot-android, from platforms/dist.json, into dist/ (or --out):
#
#   python3 platforms/scripts/build_dist.py build [aar drivers-sdk src] [--draft] [--out dist] [--aar <file>]
#   python3 platforms/scripts/build_dist.py manifest      MANIFEST.json of the repository
#   python3 platforms/scripts/build_dist.py verify [--out dist]
#
# Before building it checks: VERSION = daas-build.yaml; libdaas installed as the lock (tools/fetch_ld/fetch_ld.sh
# --check); the drivers_kit of every entry (android-drivers-sdk/build_drivers_kit.py --check) and the SHA-256 of blew
# in the drivers lock; every versioned document of docs/shared/ carries the code of VERSION and its PDF in
# docs/shared/latest/ is not older than it. --draft turns the checks into warnings and leaves the packages in
# dist/stages/draft/ (never published). The AAR is made by build_aar.py (or given with --aar). No package holds the
# sources of a specialized driver (driver_sources of dist.json): the drivers-sdk carries them compiled.
#
# Contributors:
#   Lorenzo Grillo
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-24  Lorenzo Grillo  creation, from build_dist.py of daasiot-macos (conventions of the daasiot-<target>
#                               repositories): packages aar, drivers-sdk, src; MANIFEST.json, SHA256SUMS, .sha256
"""Build, describe and check the packages of daasiot-android."""

import argparse
import datetime
import fnmatch
import glob
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tarfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
with open(os.path.join(ROOT, "platforms", "dist.json"), encoding="utf-8") as _fh:
    DIST = json.load(_fh)
COMPONENT = os.path.join(ROOT, DIST["component"])
SDK = os.path.join(ROOT, "android-drivers-sdk")
LATEST = os.path.join(ROOT, "docs", "shared", "latest")


def say(msg):
    print("[dist] " + msg, flush=True)


def problem(msg, draft):
    if draft:
        say("warning: " + msg)
    else:
        sys.exit("[dist] " + msg)


def load(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def version():
    with open(os.path.join(ROOT, "VERSION"), encoding="utf-8") as fh:
        return fh.read().strip()


def doc_code(v):
    a, b, c = (int(x) for x in v.split("."))
    return "%03d%03d%02d" % (a, b, c)


def git(*args):
    return subprocess.run(["git"] + list(args), cwd=ROOT, stdout=subprocess.PIPE, universal_newlines=True,
                          check=True).stdout


def checks(draft):
    v = version()
    if not re.fullmatch(r"\d+\.\d+\.\d+", v):
        problem("VERSION %s is not M.m.p" % v, draft)
    with open(os.path.join(ROOT, "daas-build.yaml"), encoding="utf-8") as fh:
        m = re.search(r"^versione:\s*([^\s#]+)", fh.read(), re.M)
    if not m or m.group(1) != v:
        problem("VERSION is %s, daas-build.yaml says %s" % (v, m.group(1) if m else "nothing"), draft)
    if subprocess.run(["bash", "tools/fetch_ld/fetch_ld.sh", "--check"], cwd=ROOT).returncode != 0:
        problem("libdaas is not installed as the lock (tools/fetch_ld/fetch_ld.sh)", draft)
    if subprocess.run([sys.executable, "android-drivers-sdk/build_drivers_kit.py", "--check"], cwd=ROOT).returncode:
        problem("the drivers_kit is missing (android-drivers-sdk/build_drivers_kit.py)", draft)
    for name, pkg in load(os.path.join(SDK, "libdaas-drivers.lock.json"))["blew"]["packages"].items():
        if not pkg["sha256"]:
            problem("blew %s: no SHA-256 in android-drivers-sdk/libdaas-drivers.lock.json" % name, draft)
    code = doc_code(v)
    for md in sorted(glob.glob(os.path.join(ROOT, "docs", "shared", "*.md"))):
        stem = os.path.basename(md)[:-3]
        m = re.search(r"_(\d{8})$", stem)
        if not m:
            continue
        if m.group(1) != code:
            problem("docs/shared/%s.md has the code %s, VERSION %s is %s" % (stem, m.group(1), v, code), draft)
        pdf_stem = stem if "_refs_" in stem else stem[:-9]
        pdf = os.path.join(LATEST, pdf_stem + ".pdf")
        if not os.path.isfile(pdf):
            problem("missing docs/shared/latest/%s.pdf (tools/docs_pdf/md2pdf.py)" % pdf_stem, draft)
        elif os.path.getmtime(pdf) < os.path.getmtime(md):
            problem("docs/shared/latest/%s.pdf is older than its .md" % pdf_stem, draft)


def ignore():
    return shutil.ignore_patterns(*DIST["build_ignore"])


def copy_top(dest):
    for f in DIST["top_files"]:
        if os.path.isfile(os.path.join(ROOT, f)):
            shutil.copy2(os.path.join(ROOT, f), dest)


def copy_docs(dest):
    os.makedirs(os.path.join(dest, "docs"), exist_ok=True)
    pdfs = sorted(glob.glob(os.path.join(LATEST, "*.pdf")))
    for f in pdfs or sorted(glob.glob(os.path.join(ROOT, "docs", "shared", "*.md"))):
        shutil.copy2(f, os.path.join(dest, "docs"))


def layout_aar(dest, v, aar, draft):
    copy_top(dest)
    if not aar or not os.path.isfile(aar):
        problem("no AAR (platforms/scripts/build_aar.py, or --aar)", draft)
    else:
        shutil.copy2(aar, os.path.join(dest, "android_daas-%s-release.aar" % v))
    for ex in DIST["examples"]:
        shutil.copytree(os.path.join(ROOT, ex), os.path.join(dest, ex), ignore=ignore())
    copy_docs(dest)
    with open(os.path.join(dest, "README.md"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("""<!-- daas © 2019–2026 Sebyone Srl -->

# daasiot-android %s — android_daas (AAR)

The AAR of android_daas: the Java API `it.sebyone.daas` (`DaasNode`), `libandroid_daas_jni.so` for
%s (libdaas, its drivers and the BLE driver on blew, linked in), the permissions of a daas node.

```kotlin
dependencies {
    implementation(files("libs/android_daas-%s-release.aar"))
}
```

Android API 30 or later. The example `examples/simple_node/` shows a node with BLE and INET4; the guides are in `docs/`.
""" % (v, ", ".join(k.split("/")[0][len("android-"):] for k in load(os.path.join(COMPONENT, "libdaas.lock.json"))["packages"]), v))


def layout_drivers_sdk(dest, v, draft):
    copy_top(dest)
    skip = {os.path.basename(s) for s in DIST["driver_sources"]}
    base = ignore()
    shutil.copytree(SDK, os.path.join(dest, "android-drivers-sdk"), ignore=lambda d, names: set(base(d, names)) | (
        {n for n in names if n in skip} if os.path.abspath(d) == os.path.abspath(SDK) else set()))
    os.makedirs(os.path.join(dest, DIST["component"]))
    shutil.copy2(os.path.join(COMPONENT, "libdaas.lock.json"), os.path.join(dest, DIST["component"]))
    shutil.copy2(os.path.join(COMPONENT, "libdaas.cmake"), os.path.join(dest, DIST["component"]))
    kit = os.path.join(COMPONENT, "drivers_kit")
    if os.path.isdir(kit):
        shutil.copytree(kit, os.path.join(dest, DIST["component"], "drivers_kit"))
    else:
        problem("drivers-sdk: no drivers_kit (android-drivers-sdk/build_drivers_kit.py)", draft)
    shutil.copytree(os.path.join(ROOT, "tools", "fetch_ld"), os.path.join(dest, "tools", "fetch_ld"), ignore=ignore())
    copy_docs(dest)
    with open(os.path.join(dest, "README.md"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("""<!-- daas © 2019–2026 Sebyone Srl -->

# daasiot-android %s — android-drivers-sdk (static)

The drivers specialized for Android, compiled, and the kit to write a new one, for the static libdaas packages of
`android_daas/libdaas.lock.json`.

| Path | Content |
|---|---|
| `android_daas/drivers_kit/<target>/static/` | the compiled drivers of every ABI: `libdaas_drv_ble.a` (BLE on blew), `libblew.a`, the Java side of blew in `java/`, `DRIVERS.txt` |
| `android-drivers-sdk/template/` | the smallest driver (slot `uartx`): the start of a new driver |
| `android-drivers-sdk/CMakeLists.txt`, `libdaas_package.cmake` | the build of a driver against `drivers_sdk/include` of the libdaas package, with the NDK |
| `android_daas/libdaas.cmake` | links libdaas, the drivers of the package and the drivers_kit (`libdaas::libdaas`) |
| `tools/fetch_ld/` | the libdaas packages of the lock, from the daas catalog |

```sh
tools/fetch_ld/fetch_ld.sh             # the libdaas packages of the lock -> android_daas/libdaas/
```

The sources of the specialized drivers are not in this package. See the guides in `docs/`.
""" % v)


def layout_src(dest, v, draft):
    files = git("ls-files", "--cached", "--others", "--exclude-standard").splitlines()
    excl = DIST["src_exclude"]
    for rel in files:
        if any(rel == e or rel.startswith(e + "/") or fnmatch.fnmatch(rel, e) or
               any(part == e for part in rel.split("/")) for e in excl):
            continue
        if any(fnmatch.fnmatch(os.path.basename(rel), s) or rel == s for s in DIST["secrets"]):
            continue
        src = os.path.join(ROOT, rel)
        if os.path.isfile(src):
            out = os.path.join(dest, rel)
            os.makedirs(os.path.dirname(out), exist_ok=True)
            shutil.copy2(src, out)


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def commit_id():
    try:
        return git("rev-parse", "--short", "HEAD").strip() + ("-dirty" if git("status", "--porcelain").strip() else "")
    except Exception:
        return "unknown"


def manifest(dest, name, variant, v, draft):
    llock = load(os.path.join(COMPONENT, "libdaas.lock.json"))
    dlock = load(os.path.join(SDK, "libdaas-drivers.lock.json"))
    m = {
        "product": DIST["product"], "package": name, "variant": variant, "version": v, "target": DIST["target"],
        "linkage": DIST["linkage"], "libdaas": llock["version"],
        "libdaas_packages": {k: p["tarball"] for k, p in llock["packages"].items()},
        "drivers": {k: kit["generic"] + kit["specialized"] for k, kit in dlock["kits"].items()},
        "blew": dlock["blew"]["version"], "draft": draft, "commit": commit_id(),
        "built": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "build_host": "%s %s" % (platform.system(), platform.machine()), "tests": "tests/run_tests.sh",
    }
    with open(os.path.join(dest, "MANIFEST.json"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("{\n" + ",\n".join("  %s: %s" % (json.dumps(k), json.dumps(x)) for k, x in m.items()) + "\n}\n")


def sha256sums(dest):
    lines = []
    for folder, _dirs, files in os.walk(dest):
        for f in sorted(files):
            rel = os.path.relpath(os.path.join(folder, f), dest).replace(os.sep, "/")
            if rel != "SHA256SUMS":
                lines.append("%s  %s" % (sha256_file(os.path.join(folder, f)), rel))
    with open(os.path.join(dest, "SHA256SUMS"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(sorted(lines, key=lambda l: l[66:])) + "\n")


def pack(stage, out, name):
    archive = os.path.join(out, name + ".tar.gz")
    with tarfile.open(archive, "w:gz") as tf:
        tf.add(stage, arcname=name)
    with open(archive + ".sha256", "w", encoding="utf-8", newline="\n") as fh:
        fh.write("%s  %s\n" % (sha256_file(archive), os.path.basename(archive)))
    return archive


def cmd_build(args):
    v = version()
    checks(args.draft)
    out = os.path.abspath(args.out)
    stages = os.path.join(out, "stages", "draft" if args.draft else "")
    os.makedirs(stages, exist_ok=True)
    aar = args.aar or os.path.join(ROOT, "out", "aar", "android_daas-%s-release.aar" % v)
    for variant in args.variants or list(DIST["packages"]):
        name = DIST["packages"][variant]["name"].format(product=DIST["product"], version=v, linkage=DIST["linkage"])
        stage = os.path.join(stages, name)
        if os.path.isdir(stage):
            shutil.rmtree(stage)
        os.makedirs(stage)
        if variant == "aar":
            layout_aar(stage, v, aar, args.draft)
        elif variant == "drivers-sdk":
            layout_drivers_sdk(stage, v, args.draft)
        else:
            layout_src(stage, v, args.draft)
        manifest(stage, name, variant, v, args.draft)
        sha256sums(stage)
        archive = pack(stage, stages if args.draft else out, name)
        say("%s" % os.path.relpath(archive, ROOT) if archive.startswith(ROOT) else archive)


def cmd_manifest(_args):
    v = version()
    llock = load(os.path.join(COMPONENT, "libdaas.lock.json"))
    m = {"product": DIST["product"], "version": v, "target": DIST["target"], "libdaas": llock["version"],
         "packages": {k: p["name"].format(product=DIST["product"], version=v, linkage=DIST["linkage"])
                      for k, p in DIST["packages"].items()}}
    with open(os.path.join(ROOT, "MANIFEST.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump(m, fh, indent=2, ensure_ascii=False)
        fh.write("\n")
    say("MANIFEST.json written")


def cmd_verify(args):
    out = os.path.abspath(args.out)
    archives = sorted(glob.glob(os.path.join(out, DIST["product"] + "-*.tar.gz")))
    if not archives:
        sys.exit("[dist] no package in " + out)
    bad = 0
    for a in archives:
        with open(a + ".sha256", encoding="utf-8") as fh:
            ok = fh.read().split()[0] == sha256_file(a)
        name = os.path.basename(a)[:-len(".tar.gz")]
        with tarfile.open(a, "r:gz") as tf:
            members = {m.name: m for m in tf.getmembers() if m.isfile()}
            sums = members.get(name + "/SHA256SUMS")
            sums_ok = sums is not None and (name + "/MANIFEST.json") in members
            if sums_ok:
                listed = set()
                for line in tf.extractfile(sums).read().decode("utf-8").splitlines():
                    h, rel = line.split("  ", 1)
                    listed.add(name + "/" + rel)
                    m = members.get(name + "/" + rel)
                    if m is None or hashlib.sha256(tf.extractfile(m).read()).hexdigest() != h:
                        sums_ok = False
                if set(members) - listed - {name + "/SHA256SUMS"}:
                    sums_ok = False
        sources = [m for m in members if "/%s/" % DIST["driver_sources"][0] in "/%s/" % m.split("/", 1)[-1]]
        if sources:
            say("%s: driver sources inside: %s" % (os.path.basename(a), ", ".join(sources[:3])))
        good = ok and sums_ok and not sources
        bad += not good
        say("%s: %s" % (os.path.basename(a), "ok" if good else "FAILED"))
    if bad:
        sys.exit(1)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest="cmd")
    b = sub.add_parser("build")
    b.add_argument("variants", nargs="*", choices=list(DIST["packages"]) + [[]])
    b.add_argument("--draft", action="store_true")
    b.add_argument("--out", default=os.path.join(ROOT, "dist"))
    b.add_argument("--aar", help="the AAR (default: out/aar/android_daas-<v>-release.aar of build_aar.py)")
    sub.add_parser("manifest")
    vf = sub.add_parser("verify")
    vf.add_argument("--out", default=os.path.join(ROOT, "dist"))
    args = p.parse_args()
    if not args.cmd:
        p.error("build, manifest or verify")
    {"build": cmd_build, "manifest": cmd_manifest, "verify": cmd_verify}[args.cmd](args)


if __name__ == "__main__":
    main()
