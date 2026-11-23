// daas © 2019–2026 Sebyone Srl — daasiot-android: the Gradle build of the repository (the android_daas module and
// the examples). An app includes only android_daas/ in its own build (README.md).
pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}
rootProject.name = "daasiot-android"
include(":android_daas")
include(":simple_node")
project(":simple_node").projectDir = file("examples/simple_node")
