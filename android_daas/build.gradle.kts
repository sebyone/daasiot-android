// daas © 2019–2026 Sebyone Srl
//
// File: build.gradle.kts — android_daas (daasiot-android)
//
// The Android library module of android_daas: the Java API it.sebyone.daas (DaasNode), the shared library
// android_daas_jni (android_daas, libdaas and its drivers, blew) built by the CMake project of daasiot-android, and the
// Java side of blew. An app uses it as a module of its build (include(":android_daas") with projectDir on this folder)
// or as the AAR it produces (./gradlew :android_daas:assembleRelease, or publishToMavenLocal).
//
// Contributors:
//   Lorenzo Grillo <l.grillo@sebyone.it>
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// Changes:
//   2026-11-23  Lorenzo Grillo  creation

plugins {
    id("com.android.library")
    `maven-publish`
}

val daasiotVersion = file("../VERSION").readText().trim()

android {
    namespace = "it.sebyone.daas"
    compileSdk = 36
    ndkVersion = "28.2.13676358"   // Android NDK r28c, the toolchain of the libdaas packages

    defaultConfig {
        minSdk = 30                // Android API 30, the minimum of the libdaas packages
        ndk { abiFilters += listOf("arm64-v8a") }   // the ABIs of libdaas/ and of blew
        consumerProguardFiles("consumer-rules.pro")
        externalNativeBuild {
            cmake { arguments += listOf("-DANDROID_STL=c++_static") }
        }
    }

    sourceSets["main"].java.srcDir("java")
    sourceSets["main"].java.srcDir("../android-drivers_sdk/third_party/blew/java")
    sourceSets["main"].manifest.srcFile("AndroidManifest.xml")

    externalNativeBuild {
        cmake {
            path = file("../CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    publishing {
        singleVariant("release") { withSourcesJar() }
    }
}

publishing {
    publications {
        create<MavenPublication>("release") {
            groupId = "it.sebyone.daas"
            artifactId = "android_daas"
            version = daasiotVersion
            afterEvaluate { from(components["release"]) }
        }
    }
}
