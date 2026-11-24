# CLAUDE.md

Guidance for Claude Code in **daasiot-android**: daas (Device as a Service) for Android — `android_daas` (Android
library module: Java API `it.sebyone.daas`, JNI binding, C++17 wrapper `Node`), the `android-drivers-sdk` (BLE driver
on blew), the example and the packages. Version in `VERSION` (0.2.0), libdaas 0.24.12 pinned in
`android_daas/libdaas.lock.json` (arm64-v8a, armeabi-v7a, x86_64). © Sebyone Srl, MPL-2.0. Siblings: daasiot-macos
(the origin of `android_daas`), daasiot-esp32, daasiot-gnu.

The repository follows the conventions of the daasiot-&lt;target&gt; repositories (`docs/developers/repo-target-and-
tarballs-conventions.md` of daasiot-esp32 / daasiot-macos). Read them before structural changes.

## Commands

```sh
tools/fetch_ld/fetch_ld.sh [--check | --verify]    # libdaas of the lock into android_daas/libdaas/<target>/static/ (gitignored)
python3 android-drivers-sdk/build_drivers_kit.py   # drivers_kit into android_daas/drivers_kit/ (gitignored); NDK from
                                                   # ANDROID_NDK_HOME, blew from the DaaS Build Console (DAAS_BUILDER_URL)
./gradlew :android_daas:assembleRelease            # the AAR with Gradle (Android SDK 36, NDK r28c, JDK 21)
./gradlew :simple_node:assembleDebug               # the example
python3 platforms/scripts/build_aar.py             # the AAR without Gradle (NDK, javac, jar, android.jar) -> out/aar/
python3 tools/docs_pdf/md2pdf.py docs/shared/*.md  # PDFs into docs/shared/latest/

# packages — ONLY when the owner asks explicitly
python3 platforms/scripts/build_dist.py manifest | build [--draft] | verify
```

The DaaS Build Console builds and publishes (`daas-build.yaml`, `platforms/daas_builder.sh`, SDK 6 of daasiot.com) on
its Debian VM with the NDK, the JDK and `android.jar` of the builder (tools `jdk`, `android-platform`), without Gradle.

## Layout

| Path | Content |
|---|---|
| `android_daas/` | the component: `build.gradle.kts`, `AndroidManifest.xml`, `consumer-rules.pro`, `java/it/sebyone/daas/`, `include/`, `src/`, `jni/`, `CMakeLists.txt`, `libdaas.cmake` (libdaas and the drivers_kit), `libdaas.lock.json`; `libdaas/` and `drivers_kit/` (gitignored) |
| `android-drivers-sdk/` | `ble/` (BLE 1.1.0, `ble_config.hpp` timings of blew), `template/` (slot `uartx`), `include/daasiot_android/ble.h`, `CMakeLists.txt`, `libdaas_package.cmake`, `libdaas-drivers.lock.json` (blew paths and SHA-256), `build_drivers_kit.py` |
| `examples/simple_node/` | an app with a daas node (BLE + INET4) |
| `platforms/` | `dist.json`, `scripts/build_aar.py`, `scripts/build_dist.py`, `daas_builder.sh` |
| `tools/` | `fetch_ld/` (2.0.1, `.sh` and `.ps1`), `docs_pdf/` |
| `docs/shared/` | `android-daasiot-devel_<MMMmmmpp>` (0.2.0 → `00000200`), PDFs in `latest/` (tracked) |

## Rules

- **libdaas only from the catalog** (`tools/fetch_ld`, SHA-256 of the lock and of the catalog). **blew only precompiled
  by the DaaS Build Console** (`ext/blew/android_<arch>`), never its sources or its archives here. Never commit
  libdaas, the drivers_kit or blew.
- **No namespaces in android_daas**: the C++ API is global, like libdaas; inside `Node` call the free functions hidden
  by members with `::`. The Java API mirrors `Node`: a new function gets a `native*` in `DaasNode`, its JNI function
  and, when JNI reaches a class or member by name, a rule in `consumer-rules.pro`.
- **The packages carry the specialized drivers compiled only** (`driver_sources` of `platforms/dist.json`, checked by
  `build_dist.py verify`).
- `libdaas.cmake`: the drivers of the kit go in `DAAS_EXTRA_DRIVERS` (before the drivers of the package and
  `libdaas_nodrivers.a`); blew is linked whole (its JNI entry points are reached only from Java).
- **Commits on a branch; publication and commit only with the explicit OK of the owner.** No packages unless asked.
- Every source starts with the Sebyone header and a Changes list (add a dated line when editing); Markdown files start
  with `<!-- daas © 2019–2026 Sebyone Srl -->`. English, terse, tables for layouts.
- Version bump: `VERSION`, `daas-build.yaml` (`versione`), the doc code and its PDF, `CHANGELOG.md`, `MANIFEST.json`
  (`build_dist.py manifest`); a new libdaas: the lock, the drivers lock, `libdaas_release_ids` of `daas-build.yaml`.

## Known state

- The AAR built without Gradle (`build_aar.py`) is not yet tried in an app; the Gradle build was the one used so far.
- `examples/simple_node` builds only with Gradle (an app, not packaged as a binary).
