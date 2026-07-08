import com.vanniktech.maven.publish.AndroidSingleVariantLibrary

plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("com.vanniktech.maven.publish") version "0.34.0"
}

group = "it.sebyone"
version = "0.2.3"

mavenPublishing {
    configure(
        AndroidSingleVariantLibrary(
            variant = "release",
            sourcesJar = true,
            publishJavadocJar = true,
        )
    )

    publishToMavenCentral()
    signAllPublications()

    coordinates("it.sebyone", "daasiot-android", version.toString())

    pom {
        name.set("Daas IoT Android SDK")
        description.set("Kotlin/JNI wrapper around the native daas (libdaas) C++ core library for Android.")
        url.set("https://github.com/sebyone/daasiot-android")
        inceptionYear.set("2024")

        licenses {
            license {
                name.set("Mozilla Public License 2.0")
                url.set("https://www.mozilla.org/en-US/MPL/2.0/")
            }
        }
        developers {
            developer {
                id.set("sebyone")
                name.set("Sebyone Srl")
                organization.set("Sebyone")
            }
        }
        scm {
            url.set("https://github.com/sebyone/daasiot-android")
            connection.set("scm:git:https://github.com/sebyone/daasiot-android.git")
            developerConnection.set("scm:git:ssh://git@github.com/sebyone/daasiot-android.git")
        }
    }
}

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