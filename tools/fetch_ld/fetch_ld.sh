#!/usr/bin/env bash
# daas © 2019–2026 Sebyone Srl
#
# File: fetch_ld.sh — daasiot-android
#
# Installs libdaas from the catalog of daasiot.com: exactly the packages listed in android_daas/libdaas.lock.json. For
# each "<target>/<linkage>" entry of the lock it takes the file name ("tarball") and the SHA-256 ("sha256") written
# there, verifies that the official source has the file (listed as available in the catalog
# https://daasiot.com/api/libdaas.php and served as a .tar.gz at the download_url of its entry,
# https://daasiot.com/api/download-core.php?file=<file>), downloads it, checks its SHA-256 against the lock and, when
# the catalog publishes it, against <file>.sha256, and unpacks it into libdaas/<target>/<linkage>/ (with drivers_sdk/,
# the headers of the driver contract, from libdaas 0.24.8). The lock is the list: a new target, another package or
# another libdaas version is a change of the lock (fetch_ld.sh --verify prints the published SHA-256 to write there).
# Needs bash, curl or wget, tar, od, sha256sum (or shasum). fetch_ld.ps1 is the same tool for Windows PowerShell.
#
#   tools/fetch_ld/fetch_ld.sh                      install every package of the lock
#   tools/fetch_ld/fetch_ld.sh --only android-arm64-v8a/static
#   tools/fetch_ld/fetch_ld.sh --verify [--only <target>/<linkage>]
#                                                    only check the official source (and print the published SHA-256)
#   tools/fetch_ld/fetch_ld.sh --check              offline check of what is installed (exit code 1 if not ok)
#   tools/fetch_ld/fetch_ld.sh --list               the libdaas packages of the catalog
#   tools/fetch_ld/fetch_ld.sh -h                   summary of the commands (also -help, --help)
#
# Contributors:
#   Paolo Lo Giacco <plogiacco@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Version: FETCH_LD_VERSION below, the same in fetch_ld.sh and fetch_ld.ps1 (one tool), shown in the heading of
# every run. Raise it at every change of the code of either (M.m.p: p a fix, m a new option or behaviour, M an
# incompatible change) and add a line to Changes; keep the two equal (daasiot-gnu checks it in tests/auto/t12).
#
# Changes:
#   2026-11-08  Paolo Lo Giacco  creation
#   2026-11-08  Paolo Lo Giacco  file names composed from version, target and linkage; verified in the catalog and
#                                at their URL before the download; --verify
#   2026-11-08  Paolo Lo Giacco  download from the download_url of the catalog; SHA-256 also checked against the
#                                published <file>.sha256
#   2026-11-09  Paolo Lo Giacco  1.0.0: version of the tool (FETCH_LD_VERSION) shown with the holder and the copyright
#                                at every run; -h/-help/--help; renamed from fetch_libdaas; fetch_ld.ps1 added
#   2026-11-09  Paolo Lo Giacco  1.0.1: fetch_ld.ps1 uses the tar.exe of Windows (from Git Bash the GNU tar of Git
#                                read C:\... as a remote host); fetch_ld.py exits 1 on an unknown option (was 2)
#   2026-11-09  Paolo Lo Giacco  1.0.2: an install keeps in INSTALLED.json the other packages of the lock still
#                                installed (.sh, .ps1 kept only the packages of the run); --check of fetch_ld.py
#                                reports a package of the lock missing from INSTALLED.json
#   2026-11-09  plogiacco@sebyone.it  1.0.3: target windows-x86_64-mingw32 (libdaas-<v>-windows-x86_64-mingw32-gcc-*)
#   2026-11-09  plogiacco@sebyone.it  2.0.0: the packages are the ones written in the lock (tarball, sha256), no file
#                                     name composed from a table of targets: any <target>/<linkage> of the lock is
#                                     installed; fetch_ld.py removed (the tool is .sh and .ps1), --version removed;
#                                     --verify prints the published SHA-256; --list shows every libdaas package
#   2026-11-23  l.grillo@sebyone.it  daasiot-android: lock android_daas/libdaas.lock.json, targets android-<ABI>
#                                    (static); same tool, FETCH_LD_VERSION 2.0.0

set -euo pipefail

FETCH_LD_VERSION="2.0.0"                        # version of this tool (see Version above)
CATALOG="https://daasiot.com/api/libdaas.php"
DOWNLOAD="/api/download-core.php?file="
SITE="https://daasiot.com"
USER_AGENT="daasiot-android-fetch-ld"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LOCK="$ROOT/android_daas/libdaas.lock.json"
DEST="$ROOT/libdaas"
STAMP="$DEST/INSTALLED.json"

log() { echo "[libdaas] $*"; }
fail() { echo "[libdaas] $*" >&2; exit 1; }

get() {   # get <url> <output file> [range]
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL -A "$USER_AGENT" ${3:+-r "$3"} -o "$2" "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -q -U "$USER_AGENT" ${3:+--header="Range: bytes=$3"} -O "$2" "$1"
    else fail "curl or wget is required"; fi
}

sha256() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | cut -d' ' -f1
    else shasum -a 256 "$1" | cut -d' ' -f1; fi
}

is_gzip() { [ "$(head -c2 "$1" | od -An -tx1 | tr -d ' \n')" = "1f8b" ]; }

# ---- the lock (JSON edited by hand: read without jq, whatever the layout) ------------------------------------------
# lock_version: the top-level "version"; lock_entries: one line "<target>/<linkage> <tarball or -> <sha256 or ->"
# per package, in the order of the lock
lock_flat() { tr -d '\r\n\t' < "$LOCK"; }
lock_version() { lock_flat | sed 's/"packages"[[:space:]]*:.*//' | sed -n 's/.*"version"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p'; }
lock_entries() {
    lock_flat | sed 's/.*"packages"[[:space:]]*:[[:space:]]*{//' | sed 's/}/}\n/g' \
        | sed -n 's/^[[:space:],]*"\([^"]*\/[^"]*\)"[[:space:]]*:[[:space:]]*{\(.*\)}.*/\1 \2/p' \
        | while read -r key rest; do
            tarball="$(echo "$rest" | sed -n 's/.*"tarball"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p')"
            digest="$(echo "$rest" | sed -n 's/.*"sha256"[[:space:]]*:[[:space:]]*"\([0-9A-Fa-f]*\)".*/\1/p' | tr A-F a-f)"
            echo "$key ${tarball:--} ${digest:--}"
        done
}
keys_of() { printf '%s\n' "$@" | cut -d' ' -f1 | paste -sd' ' -; }

CATALOG_JSON=""
catalog_entries() {   # one release object per line
    if [ -z "$CATALOG_JSON" ]; then
        CATALOG_JSON="$WORK/catalog.json"
        get "$CATALOG" "$CATALOG_JSON" || fail "catalog not reachable: $CATALOG"
    fi
    sed 's/},{/}\n{/g' "$CATALOG_JSON"
}

abs_url() { case "$1" in http*) echo "$1" ;; *) echo "$SITE$1" ;; esac; }

# verify <name>: prints "<url> <checksum url or ->" if the official source has the package, else a reason on stderr
verify() {
    local name="$1" entry url sha
    entry="$(catalog_entries | grep -F "\"file\":\"$name\"" | head -1 || true)"
    [ -n "$entry" ] || { log "$name is not in the catalog $CATALOG" >&2; return 1; }
    echo "$entry" | grep -q '"available":true' || { log "$name is listed in the catalog but marked not available" >&2; return 1; }
    url="$(echo "$entry" | sed -n 's/.*"download_url":"\([^"]*\)".*/\1/p' | sed 's#\\/#/#g')"
    [ -n "$url" ] || url="$DOWNLOAD$name"
    url="$(abs_url "$url")"
    sha="-"
    if echo "$entry" | grep -q '"sig_available":true' && echo "$entry" | grep -qF "\"signature\":\"$name.sha256\""; then
        sha="$(echo "$entry" | sed -n 's/.*"sig_url":"\([^"]*\)".*/\1/p' | sed 's#\\/#/#g')"
        [ -n "$sha" ] || sha="$DOWNLOAD$name.sha256"
        sha="$(abs_url "$sha")"
    fi
    rm -f "$WORK/probe"
    if ! get "$url" "$WORK/probe" 0-1 2>/dev/null; then
        log "$name is listed as available but $url does not serve it" >&2; return 1
    fi
    is_gzip "$WORK/probe" || { log "$name: $url does not return a .tar.gz" >&2; return 1; }
    echo "$url $sha"
}

published_sha256() {   # published_sha256 <checksum url> <name>: the SHA-256 the catalog publishes for the file
    get "$1" "$WORK/$2.sha256" || return 1
    cut -d' ' -f1 < "$WORK/$2.sha256" | tr -d '\r\n' | tr A-F a-f
}

usage() {
    cat <<'EOF'
fetch_ld.sh: libdaas for daasiot-android from the catalog of daasiot.com, the packages of android_daas/libdaas.lock.json

usage: tools/fetch_ld/fetch_ld.sh [command] [options]

commands:
  (none)                      verify, download and install every package of the lock into libdaas/
  --verify                    only check that the catalog has the packages of the lock and serves them, and
                              compare the published SHA-256 with the lock (nothing installed)
  --check                     offline check of what is installed (exit code 1 if not ok)
  --list                      the libdaas packages of the catalog
  -h, -help, --help           this summary

options:
  --only <target>/<linkage>   one package of the lock (repeatable), e.g. --only android-arm64-v8a/static

the lock: "version" and, under "packages", one "<target>/<linkage>": { "tarball": "<file of the catalog>",
"sha256": "<its SHA-256>" } per package; edit it to add a target or change the version (--verify gives the SHA-256)
exit code: 0 done, 1 a check or a download failed
EOF
}

# heading of every run: the tool, the release of daasiot-android, the holder and the copyright
banner() {
    local v
    v="$(tr -d ' \r\n' < "$ROOT/VERSION" 2>/dev/null || true)"
    echo "fetch_ld $FETCH_LD_VERSION (daasiot-android${v:+ $v}): libdaas from the catalog of daasiot.com"
    echo "daas © 2019–2026 Sebyone Srl (sebyone.it), holder of daas and of daasiot-android. MPL-2.0."
    echo
}

banner
for a in "$@"; do case "$a" in -h|-help|--help) usage; exit 0 ;; esac; done
[ -f "$LOCK" ] || fail "lock not found: $LOCK"
MODE="install"
ONLY=()
while [ $# -gt 0 ]; do
    case "$1" in
        --check) MODE="check" ;;
        --list) MODE="list" ;;
        --verify) MODE="verify" ;;
        --only) shift; ONLY+=("${1:?--only needs <target>/<linkage>}") ;;
        *) fail "unknown option $1 (-h: the commands)" ;;
    esac
    shift
done
VERSION="$(lock_version)"
[ -n "$VERSION" ] || fail "no \"version\" in $LOCK"
mapfile -t ALL < <(lock_entries)
[ ${#ALL[@]} -gt 0 ] || fail "no package in $LOCK"
if [ ${#ONLY[@]} -gt 0 ]; then
    ENTRIES=()
    for key in "${ONLY[@]}"; do
        e="$(printf '%s\n' "${ALL[@]}" | awk -v k="$key" '$1 == k { print; exit }')"
        [ -n "$e" ] || fail "$key is not in the lock android_daas/libdaas.lock.json ($(keys_of "${ALL[@]}"))"
        ENTRIES+=("$e")
    done
else
    ENTRIES=("${ALL[@]}")
fi
for e in "${ENTRIES[@]}"; do
    read -r key name digest <<< "$e"
    [ "$name" != "-" ] || fail "$key: no \"tarball\" in the lock"
done
WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT

case "$MODE" in
list)
    log "catalog $CATALOG (lock: libdaas $VERSION)"
    catalog_entries | sed -n 's/.*"file":"\(libdaas-[^"]*\.tar\.gz\)".*"available":\([a-z]*\).*/  \1  (available: \2)/p' | sort -ru
    ;;
check)
    problems=0
    installed=""
    [ -f "$STAMP" ] && installed="$(tr -d '\r\n' < "$STAMP" | sed 's/"packages".*//' | sed -n 's/.*"version"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p')"
    for e in "${ENTRIES[@]}"; do
        read -r key name digest <<< "$e"
        dir="$DEST/$key"
        if [ ! -f "$dir/include/daas.hpp" ]; then log "missing libdaas/$key/include/daas.hpp"; problems=1
        elif [ ! -f "$dir/lib/libdaas.a" ] && [ ! -f "$dir/lib/libdaas.so" ]; then log "missing libdaas/$key/lib/libdaas.*"; problems=1; fi
        if [ -f "$STAMP" ] && ! grep -qF "\"$name\"" "$STAMP"; then log "libdaas/$key was not installed from $name"; problems=1; fi
    done
    [ "$installed" = "$VERSION" ] || { log "installed libdaas ${installed:-unknown}, required $VERSION"; problems=1; }
    [ $problems -eq 0 ] || { log "run: tools/fetch_ld/fetch_ld.sh"; exit 1; }
    log "libdaas $VERSION installed: $(keys_of "${ENTRIES[@]}")"
    ;;
verify|install)
    log "libdaas $VERSION: verifying ${#ENTRIES[@]} package(s) of the lock in $CATALOG"
    declare -A URL=() SHA_URL=()
    missing=0
    for e in "${ENTRIES[@]}"; do
        read -r key name digest <<< "$e"
        if found="$(verify "$name")"; then URL[$key]="${found% *}"; SHA_URL[$key]="${found##* }"; log "$key: $name found"; else missing=1; fi
    done
    if [ "$MODE" = "verify" ]; then
        for e in "${ENTRIES[@]}"; do
            read -r key name digest <<< "$e"
            [ -n "${SHA_URL[$key]:-}" ] && [ "${SHA_URL[$key]}" != "-" ] || continue
            published="$(published_sha256 "${SHA_URL[$key]}" "$name" || true)"
            if [ -z "$published" ]; then log "$key: published checksum not readable"; missing=1
            elif [ "$digest" = "-" ]; then log "$key: no sha256 in the lock; published SHA-256 of $name: $published"; missing=1
            elif [ "$digest" != "$published" ]; then log "$key: the lock has SHA-256 $digest, the catalog publishes $published"; missing=1
            else log "$key: the SHA-256 of the lock is the published one"; fi
        done
        [ $missing -eq 0 ] || exit 1
        log "every package of the lock is in the official source"; exit 0
    fi
    [ $missing -eq 0 ] || fail "nothing installed: the official source does not have every package (see above)"

    entries=""
    for e in "${ENTRIES[@]}"; do
        read -r key name expected <<< "$e"
        [ "$expected" != "-" ] || fail "$key: no \"sha256\" in the lock: write the published one (tools/fetch_ld/fetch_ld.sh --verify --only $key)"
        get "${URL[$key]}" "$WORK/$name" || fail "download failed: ${URL[$key]}"
        is_gzip "$WORK/$name" || fail "$name is not a .tar.gz archive"
        digest="$(sha256 "$WORK/$name")"
        [ "$digest" = "$expected" ] || fail "$name: SHA-256 $digest does not match the lock ($expected)"
        if [ "${SHA_URL[$key]}" != "-" ]; then
            published="$(published_sha256 "${SHA_URL[$key]}" "$name")" || fail "checksum not readable: ${SHA_URL[$key]}"
            [ "$digest" = "$published" ] || fail "$name: SHA-256 $digest does not match the published checksum ($published)"
            log "$key: SHA-256 matches the lock and the published $name.sha256"
        fi
        if tar -tzf "$WORK/$name" | grep -Eq '(^/|(^|/)\.\.(/|$))'; then fail "unsafe path in $name"; fi
        rm -rf "$WORK/x" && mkdir -p "$WORK/x" && tar -xzf "$WORK/$name" -C "$WORK/x"
        src="$WORK/x"
        tops=("$WORK/x"/*)
        if [ ${#tops[@]} -eq 1 ] && [ -d "${tops[0]}" ] && [ ! -d "$WORK/x/include" ]; then src="${tops[0]}"; fi
        [ -f "$src/include/daas.hpp" ] || fail "$key: include/daas.hpp missing: not a libdaas package"
        pkg="$(tr -d ' \r\n' < "$src/VERSION.txt" 2>/dev/null || true)"
        [ -z "$pkg" ] || [ "$pkg" = "$VERSION" ] || fail "$key: package version $pkg is not the $VERSION of the lock"
        rm -rf "${DEST:?}/$key" && mkdir -p "$DEST/$key" && cp -R "$src"/. "$DEST/$key"/
        log "$key: $name -> libdaas/$key/"
        entries="$entries${entries:+,}
    \"$key\": { \"tarball\": \"$name\", \"sha256\": \"$digest\" }"
    done
    # the other packages of the lock still installed from this version stay in the record
    if [ -f "$STAMP" ] && grep -q "\"version\": \"$VERSION\"" "$STAMP"; then
        done_keys=" $(keys_of "${ENTRIES[@]}") "
        for e in "${ALL[@]}"; do
            read -r key name digest <<< "$e"
            case "$done_keys" in *" $key "*) continue ;; esac
            grep -qF "\"$name\"" "$STAMP" && [ -f "$DEST/$key/include/daas.hpp" ] || continue
            entries="$entries${entries:+,}
    \"$key\": { \"tarball\": \"$name\", \"sha256\": \"$digest\" }"
        done
    fi
    mkdir -p "$DEST"
    cat > "$STAMP" <<EOF
{
  "version": "$VERSION",
  "source": "$CATALOG",
  "packages": {$entries
  }
}
EOF
    log "done"
    ;;
esac
