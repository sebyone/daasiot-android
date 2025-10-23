plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
}

group = "sebyone.daasiot"
version = "0.0.2"

android {
    namespace = "sebyone.daasiot_android"
    compileSdk = 36

    defaultConfig {
        minSdk = 24

        // ✅ Removed versionCode / versionName — not valid in libraries
        // ✅ targetSdk moved below
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndk {
            abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86_64"))
        }
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

    buildFeatures {
        viewBinding = true
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
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}