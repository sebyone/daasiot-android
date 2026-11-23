// daas © 2019–2026 Sebyone Srl
//
// File: build.gradle.kts — examples/simple_node (daasiot-android)
//
// A daas node in an Android app: android_daas as a dependency (black box: only the API it.sebyone.daas).
// Built with the repository (./gradlew :simple_node:assembleDebug). Copied out of it, use the AAR of android_daas
// (implementation(files("android_daas-<v>-release.aar"))) or include the module (README.md).
//
// Contributors:
//   Lorenzo Grillo <l.grillo@sebyone.it>
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0 (this example is a starting
// point: license your own project as you wish).
//
// Changes:
//   2026-11-23  Lorenzo Grillo  creation

plugins {
    id("com.android.application")
}

android {
    namespace = "it.sebyone.daas.example"
    compileSdk = 36

    defaultConfig {
        applicationId = "it.sebyone.daas.example"
        minSdk = 30
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"
        ndk { abiFilters += listOf("arm64-v8a") }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    implementation(project(":android_daas"))
}
