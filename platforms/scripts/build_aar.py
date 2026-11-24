#!/usr/bin/env python3
# daas © 2019–2026 Sebyone Srl
#
# File: build_aar.py — daasiot-android (platforms/scripts)
#
# Builds the AAR of android_daas without Gradle (the DaaS Build Console has no Android SDK, only the NDK, a JDK and
# android.jar): for every ABI of android_daas/libdaas.lock.json the CMake project of the repository with the toolchain
# of the NDK (libandroid_daas_jni.so: android_daas, libdaas, its drivers and the drivers_kit), then the Java of
# android_daas and of blew (the java/ of the drivers_kit) compiled against android.jar, and the AAR:
#
#   AndroidManifest.xml (package it.sebyone.daas, minSdk 30), classes.jar, jni/<abi>/libandroid_daas_jni.so,
#   proguard.txt (consumer-rules.pro), R.txt
#
#   python3 platforms/scripts/build_aar.py --out out/aar [--ndk <ndk>] [--android-jar <android.jar>] [--only <abi>]
#
# The NDK: --ndk, ANDROID_NDK_HOME or ANDROID_NDK_ROOT. android.jar: --android-jar, ANDROID_JAR, the output of
# `android-jar` (the tool of the builder) or $ANDROID_HOME/platforms/android-36/android.jar. Needs cmake, javac, jar.
# Python 3.7 or later.
#
# Contributors:
#   Lorenzo Grillo
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-24  Lorenzo Grillo  creation: the AAR of android_daas on the DaaS Build Console
"""Build the AAR of android_daas with the NDK, javac and jar (no Gradle)."""

import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
COMPONENT = os.path.join(ROOT, "android_daas")
API_LEVEL = "30"            # minimum Android API of the libdaas packages
PACKAGE = "it.sebyone.daas"


def say(msg):
    print("[aar] " + msg, flush=True)


def fail(msg):
    sys.exit("[aar] " + msg)


def run(cmd, **kw):
    subprocess.run(cmd, check=True, **kw)


def find_ndk(given):
    for ndk in (given, os.environ.get("ANDROID_NDK_HOME"), os.environ.get("ANDROID_NDK_ROOT")):
        if ndk and os.path.isfile(os.path.join(ndk, "build", "cmake", "android.toolchain.cmake")):
            return os.path.abspath(ndk)
    fail("Android NDK not found: --ndk or ANDROID_NDK_HOME")


def find_android_jar(given):
    cands = [given, os.environ.get("ANDROID_JAR")]
    if shutil.which("android-jar"):
        cands.append(subprocess.run(["android-jar"], stdout=subprocess.PIPE, universal_newlines=True).stdout.strip())
    if os.environ.get("ANDROID_HOME"):
        cands += sorted(glob.glob(os.path.join(os.environ["ANDROID_HOME"], "platforms", "android-*", "android.jar")),
                        reverse=True)
    for c in cands:
        if c and os.path.isfile(c):
            return os.path.abspath(c)
    fail("android.jar not found: --android-jar, ANDROID_JAR or ANDROID_HOME")


def strip_tool(ndk):
    found = glob.glob(os.path.join(ndk, "toolchains", "llvm", "prebuilt", "*", "bin", "llvm-strip"))
    return found[0] if found else None


def manifest(version):
    """The manifest of the module, with the package and the minimum SDK an AAR declares."""
    with open(os.path.join(COMPONENT, "AndroidManifest.xml"), encoding="utf-8") as fh:
        text = fh.read()
    if "package=" not in text:
        text = text.replace('<manifest xmlns:android="http://schemas.android.com/apk/res/android"',
                            '<manifest xmlns:android="http://schemas.android.com/apk/res/android"\n'
                            '    package="%s" android:versionName="%s"' % (PACKAGE, version), 1)
    if "<uses-sdk" not in text:
        text = re.sub(r"(<manifest[^>]*>)", r'\1\n    <uses-sdk android:minSdkVersion="%s" />' % API_LEVEL, text, count=1)
    return text


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--out", default=os.path.join(ROOT, "out", "aar"))
    p.add_argument("--ndk")
    p.add_argument("--android-jar")
    p.add_argument("--only", action="append", metavar="ABI")
    p.add_argument("--cmake-arg", action="append", default=[])
    args = p.parse_args()

    with open(os.path.join(ROOT, "VERSION"), encoding="utf-8") as fh:
        version = fh.read().strip()
    with open(os.path.join(COMPONENT, "libdaas.lock.json"), encoding="utf-8") as fh:
        abis = [k.split("/")[0][len("android-"):] for k in json.load(fh)["packages"]]
    if args.only:
        abis = [a for a in abis if a in args.only] or fail("no ABI of the lock in --only %s" % args.only)
    ndk, android_jar = find_ndk(args.ndk), find_android_jar(args.android_jar)
    cmake = shutil.which("cmake") or fail("cmake not found")
    javac, jar = shutil.which("javac") or fail("javac not found"), shutil.which("jar") or fail("jar not found")
    out = os.path.abspath(args.out)
    work = os.path.join(out, "work")
    if os.path.isdir(work):
        shutil.rmtree(work)
    os.makedirs(work)

    # 1. native: libandroid_daas_jni.so per ABI
    libs = {}
    strip = strip_tool(ndk)
    for abi in abis:
        build = os.path.join(work, "native", abi)
        say("%s: libandroid_daas_jni.so" % abi)
        run([cmake, "-S", ROOT, "-B", build,
             "-DCMAKE_TOOLCHAIN_FILE=" + os.path.join(ndk, "build", "cmake", "android.toolchain.cmake"),
             "-DANDROID_ABI=" + abi, "-DANDROID_PLATFORM=android-" + API_LEVEL, "-DANDROID_STL=c++_static",
             "-DCMAKE_BUILD_TYPE=Release"] + args.cmake_arg, stdout=subprocess.DEVNULL)
        run([cmake, "--build", build, "--target", "android_daas_jni", "-j"])
        so = os.path.join(build, "android_daas", "libandroid_daas_jni.so")
        if not os.path.isfile(so):
            fail("%s: %s not built" % (abi, so))
        if strip:
            run([strip, "--strip-unneeded", so])
        libs[abi] = so

    # 2. Java: android_daas and the Java side of blew (the same in the kit of every ABI)
    sources = sorted(glob.glob(os.path.join(COMPONENT, "java", "**", "*.java"), recursive=True))
    blew_java = os.path.join(COMPONENT, "drivers_kit", "android-%s" % abis[0], "static", "java")
    if os.path.isdir(blew_java):
        sources += sorted(glob.glob(os.path.join(blew_java, "**", "*.java"), recursive=True))
    else:
        fail("no Java of blew in %s: build the drivers_kit (android-drivers-sdk/build_drivers_kit.py)" % blew_java)
    classes = os.path.join(work, "classes")
    os.makedirs(classes)
    say("javac: %d sources against %s" % (len(sources), android_jar))
    run([javac, "--release", "11", "-encoding", "UTF-8", "-nowarn", "-classpath", android_jar, "-d", classes] + sources)
    classes_jar = os.path.join(work, "classes.jar")
    run([jar, "cf", classes_jar, "-C", classes, "."])

    # 3. the AAR
    os.makedirs(out, exist_ok=True)
    aar = os.path.join(out, "android_daas-%s-release.aar" % version)
    with zipfile.ZipFile(aar, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("AndroidManifest.xml", manifest(version))
        zf.write(classes_jar, "classes.jar")
        for abi, so in sorted(libs.items()):
            zf.write(so, "jni/%s/libandroid_daas_jni.so" % abi)
        zf.write(os.path.join(COMPONENT, "consumer-rules.pro"), "proguard.txt")
        zf.writestr("R.txt", "")
    say("%s (%s)" % (os.path.relpath(aar, ROOT) if aar.startswith(ROOT) else aar, ", ".join(sorted(libs))))


if __name__ == "__main__":
    main()
