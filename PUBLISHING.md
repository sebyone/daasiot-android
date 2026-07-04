# Publishing to Maven Central

This guide describes how to publish `daasiotsdk` as a proper Maven artifact on **Maven Central**
(the public, official Maven repository), so consuming apps can declare it as a normal dependency
instead of copying the `.aar` file into `libs/`:

```kotlin
dependencies {
    implementation("io.github.<namespace>:daasiotsdk:0.2.0") // instead of files("libs/...")
}
```

Nothing in this guide has been applied to the project yet — `build.gradle.kts` still only produces
a local `.aar` (see [Installation](README.md#installation) and [RELEASING.md](RELEASING.md)). Follow
the steps below when you're ready to make the first public release.

## 0. Decide and verify a namespace (blocking prerequisite)

Maven Central requires you to prove ownership of the **groupId's namespace** before it will accept
anything published under it. The current groupId in `daasiotsdk/build.gradle.kts` is
`sebyone.daasiot`, which **will not pass verification as-is** — it's neither a verified reverse
domain nor a `io.github.*` namespace. Pick one path before doing anything else:

- **Reverse-domain namespace** (e.g. `com.sebyone`, if the company owns `sebyone.com`): verified by
  adding a DNS `TXT` record to that domain, or by adding a temporary file/README marker the Central
  Portal asks for. Requires DNS access to the domain.
- **`io.github.<username>` namespace** (e.g. `io.github.sebyone` or `io.github.<personal-account>`):
  verified automatically by logging into the Central Portal with that GitHub account — no DNS
  access needed. This is the simpler path if there's no company-owned domain handy, but it ties the
  coordinates to a specific GitHub account/org rather than the company's own domain.

Whichever you choose, `group` in `daasiotsdk/build.gradle.kts` must be updated to match (this is a
breaking change for anyone already depending on the current `sebyone.daasiot` coordinates via the
manual `.aar` copy — bump to a new major/minor version and call it out in `CHANGELOG.md`).

## 1. Create a Central Portal account

1. Sign up at https://central.sonatype.com (this replaced the legacy OSSRH/`s01.oss.sonatype.org`
   flow, which Sonatype has fully sunset).
2. Register the namespace chosen in step 0 and complete its verification.
3. Generate a **user token** (Account → Generate User Token) — this gives you a
   `mavenCentralUsername` / `mavenCentralPassword` pair. This is *not* your account login password;
   it's a scoped token you can revoke independently.

## 2. Generate a GPG signing key

Every artifact uploaded to Central (POM, `.aar`, sources jar, javadoc jar) must be signed with GPG.

```bash
gpg --full-generate-key           # RSA 4096, no expiry or a long one — losing this key later means you can never publish under it again
gpg --list-secret-keys --keyid-format LONG   # note the key ID (the part after "rsa4096/")
gpg --keyserver keyserver.ubuntu.com --send-keys <KEY_ID>
gpg --keyserver keys.openpgp.org --send-keys <KEY_ID>
```

Central's verification checks the key against public keyservers, so publish it to at least one
(preferably two, since propagation between keyservers can lag).

Export the private key in ASCII-armored form for use in signing (see step 4):

```bash
gpg --export-secret-keys --armor <KEY_ID> > central-signing-key.asc
```

Treat `central-signing-key.asc` and the Central Portal user token like credentials — never commit
them to the repo. Store them in `~/.gradle/gradle.properties` (outside the project) or as CI
secrets, never in project-local `gradle.properties`.

## 3. Add the publishing plugin

Publishing an Android library (AAR) to Central by hand — building the bundle zip, checksums,
signatures, and uploading via the raw Publisher API — is fiddly and easy to get subtly wrong. Use
the community-maintained
[`com.vanniktech.maven.publish`](https://github.com/vanniktech/gradle-maven-publish-plugin) Gradle
plugin, which has first-class support for both Android library components and the Central Portal.
Check its README for the current plugin version and exact DSL — the snippet below reflects the
shape of the API at the time of writing, but this plugin evolves and the specific method names
(e.g. `publishToMavenCentral()`) have changed across major versions.

In `daasiotsdk/build.gradle.kts`:

```kotlin
plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("com.vanniktech.maven.publish") version "0.30.0" // check for a newer version
}

mavenPublishing {
    publishToMavenCentral()
    signAllPublications()

    coordinates("io.github.<namespace>", "daasiotsdk", version.toString())

    pom {
        name.set("Daas IoT Android SDK")
        description.set("Kotlin/JNI wrapper around the native daas (libdaas) C++ core library for Android.")
        url.set("https://github.com/<org>/daasiot-android")
        inceptionYear.set("2024")

        licenses {
            license {
                name.set("Mozilla Public License 2.0")
                url.set("https://www.mozilla.org/en-US/MPL/2.0/")
            }
        }
        developers {
            developer {
                id.set("<github-username>")
                name.set("<full name>")
                organization.set("Sebyone")
            }
        }
        scm {
            url.set("https://github.com/<org>/daasiot-android")
            connection.set("scm:git:https://github.com/<org>/daasiot-android.git")
            developerConnection.set("scm:git:ssh://git@github.com/<org>/daasiot-android.git")
        }
    }
}
```

`version`/`group` stay defined where they already are (top of the same file); the plugin reads
them, or you can pass them explicitly to `coordinates(...)` as above.

### Credentials

In `~/.gradle/gradle.properties` (not the project's own `gradle.properties`):

```properties
mavenCentralUsername=<user token username from step 1>
mavenCentralPassword=<user token password from step 1>
signingInMemoryKey=<contents of central-signing-key.asc, or a path depending on plugin version>
signingInMemoryKeyPassword=<the GPG key's passphrase>
```

## 4. Verify locally before publishing

```powershell
./gradlew :daasiotsdk:publishToMavenLocal
```

Then inspect `~/.m2/repository/io/github/<namespace>/daasiotsdk/<version>/` — confirm the `.aar`,
`-sources.jar`, `-javadoc.jar`, `.pom`, and their `.asc` signature files are all present before
attempting a real upload (Central rejects incomplete bundles, and failed attempts still count
against you if the namespace has upload-rate limits).

## 5. Publish

```powershell
./gradlew :daasiotsdk:publishToMavenCentral
```

Depending on the plugin/version, this either:
- auto-releases the deployment once validation passes, or
- uploads a "deployment bundle" that needs a manual **Publish** click in the Central Portal web UI
  (Deployments tab) after you review the validation report.

## 6. Confirm the release

Artifacts typically sync to https://search.maven.org within 10–30 minutes of release. Until then
they're visible only in the Central Portal's own deployment view.

## 7. Versioning rules (Central-specific, not just good practice)

- **Published versions are immutable.** You cannot delete or overwrite `0.2.0` once it's live —
  fixing anything means publishing `0.2.1`. Don't attempt a first Central publish from a version
  number you're not fully done testing.
- Keep `CHANGELOG.md` and the README's `Installation` section (currently describing the manual
  `.aar` copy) updated to point at the new Maven coordinates once the first release is live.

## After the first successful release

Update:
- `README.md` → `Installation` section: replace the manual AAR-copy instructions with the Gradle
  dependency coordinates.
- `RELEASING.md` → replace the stale `uploadArchives` step with `./gradlew publishToMavenCentral`
  (see cross-reference already added there).
