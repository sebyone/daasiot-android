#!/bin/bash
# daas © 2019–2026 Sebyone Srl — MPL-2.0
#
# File: daas_builder.sh — daasiot-android (platforms)
#
# Build script of the DaaS Build Console (daas-builder, daas-build.yaml at the repository root), on its Debian VM with
# the Android NDK ($ANDROID_NDK_HOME), a JDK and android.jar (tools jdk and android-platform of the builder): installs
# libdaas from the catalog of daasiot.com (tools/fetch_ld/fetch_ld.sh, the packages of android_daas/libdaas.lock.json),
# builds the drivers_kit (android-drivers-sdk/build_drivers_kit.py, blew from the console), the AAR without Gradle
# (platforms/scripts/build_aar.py), writes MANIFEST.json and builds the packages into $OUT
# (platforms/scripts/build_dist.py). It runs in the builder's copy of the repository, never in a developer checkout.
#
# Contributors:
#   Lorenzo Grillo
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-24  Lorenzo Grillo  creation (conventions of the daasiot-<target> repositories)
set -euo pipefail
: "${OUT:?set by daas-builder}"
: "${ANDROID_NDK_HOME:?the Android NDK of the VM (config.yaml of the builder)}"

# Python 3.8 or later when there is one (the Debian VM has 3.7 as python3; the scripts accept 3.7)
PY=""
for p in python3.12 python3.11 python3.10 python3.9 python3.8 python3; do
    if command -v "$p" >/dev/null && "$p" -c 'import sys; sys.exit(sys.version_info < (3, 7))'; then
        PY="$p"; break
    fi
done
[ -n "$PY" ] || { echo "daas_builder: Python 3.7 or later not found"; exit 1; }

v="$(tr -d '[:space:]' < VERSION)"
m="$(sed -n 's/^versione: *\([^ #]*\).*/\1/p' daas-build.yaml)"
[ "$v" = "$m" ] || echo "daas_builder: warning: VERSION is $v, daas-build.yaml says $m"

echo "== libdaas from the catalog (android_daas/libdaas.lock.json)"
bash tools/fetch_ld/fetch_ld.sh

# blew from the DaaS Build Console: in the VM 127.0.0.1 is the VM itself, the console listens on the LAN of nobile
# («indirizzo» of config.yaml of the builder). DAAS_BUILDER_URL, when set, comes first.
if [ -z "${DAAS_BUILDER_URL:-}" ]; then
    for u in http://127.0.0.1:8095 http://192.168.2.231:8095; do
        if "$PY" -c 'import sys, urllib.request; urllib.request.urlopen(sys.argv[1], timeout=5)' "$u/" 2>/dev/null; then
            export DAAS_BUILDER_URL="$u"; break
        fi
    done
fi
[ -n "${DAAS_BUILDER_URL:-}" ] || { echo "daas_builder: the DaaS Build Console is not reachable (DAAS_BUILDER_URL)"; exit 1; }
echo "== drivers_kit (blew from $DAAS_BUILDER_URL)"
"$PY" android-drivers-sdk/build_drivers_kit.py

echo "== AAR"
"$PY" platforms/scripts/build_aar.py --out out/aar

echo "== daasiot-android $v packages"
"$PY" platforms/scripts/build_dist.py manifest
"$PY" platforms/scripts/build_dist.py build --out "$OUT" --aar "out/aar/android_daas-$v-release.aar"
"$PY" platforms/scripts/build_dist.py verify --out "$OUT"
rm -rf "$OUT/stages"
ls -l "$OUT"
