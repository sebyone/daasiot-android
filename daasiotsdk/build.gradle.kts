plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
}

group = "sebyone.daasiot"
version = "0.2.0"

android {
    namespace = "sebyone.daasiot_android"
    compileSdk = 36

    defaultConfig {
        minSdk = 30

        // ✅ Removed versionCode / versionName — not valid in libraries
        // ✅ targetSdk moved below
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndk {
//            abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86_64"))
            abiFilters.add("arm64-v8a")

        }

        // Bundled into the AAR and applied automatically by any consuming app that enables R8/
        // minification - see consumer-rules.pro for why this is required (JNI resolves classes
        // and members by exact name, invisibly to R8's static analysis).
        consumerProguardFiles("consumer-rules.pro")
    }

    // ✅ New AGP 8+ locations for targetSdk
    lint {
        targetSdk = 36
    }
    testOptions {
        targetSdk = 36
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    // ✅ Modern Kotlin DSL for compiler options
    kotlin {
        compilerOptions {
            jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_11)
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    // ✅ Modern output naming for AGP 8+
    publishing {
        singleVariant("release")
    }

}


// ✅ Rename AAR after assembleRelease
val libVersion = version.toString()

afterEvaluate {
    tasks.named("assembleRelease").configure {
        doLast {
            val buildDir = layout.buildDirectory.asFile.get()
            val aarFile = file("$buildDir/outputs/aar/daasiotsdk-release.aar")
            if (aarFile.exists()) {
                val newName = "daasiotsdk-${libVersion}-release.aar"
                aarFile.renameTo(File(aarFile.parentFile, newName))
                println("✅ Renamed AAR to: $newName")
            }
        }
    }
}

dependencies {
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}