# Consumer ProGuard/R8 rules, bundled into the AAR and applied automatically by any app that
# depends on this library and enables minification. Without these, R8 in the *consuming app* is
# free to rename or strip anything below, and the native layer would fail at runtime -
# ClassNotFoundException / NoSuchMethodError - since it locates these classes/members by exact
# name via JNI (FindClass/GetMethodID), not by static linkage R8 can see and account for.

# DaasWrapper/DDO's `external fun` native methods are resolved by the JVM at class-load time via
# implicit JNI registration, which mangles the *fully-qualified class name* into the expected
# native symbol (e.g. Java_sebyone_daasiot_1android_DaasWrapper_nativeCreate). If either class is
# renamed, that symbol lookup silently fails with UnsatisfiedLinkError.
-keepclasseswithmembers class sebyone.daasiot_android.DaasWrapper {
    native <methods>;
}
-keepclasseswithmembers class sebyone.daasiot_android.entity.DDO {
    native <methods>;
}

# Data classes the native layer constructs via FindClass + GetMethodID("<init>", ...) with a
# hardcoded JNI type descriptor - the class, its constructor signature, and its members must stay
# exactly as compiled.
-keep class sebyone.daasiot_android.entity.NodeInfo { *; }
-keep class sebyone.daasiot_android.entity.NodeNetworkInfo { *; }
-keep class sebyone.daasiot_android.entity.NodeMapEntry { *; }
-keep class sebyone.daasiot_android.entity.PullResult { *; }
-keep class sebyone.daasiot_android.entity.AvailablePullResult { *; }
-keep class sebyone.daasiot_android.entity.FeatureRequest { *; }

# Every native enum (DaasError, LinkType, PerformMode, AcceptRequestPolicy, DDOPolicy,
# DiscoveryState, StreamType, Feature, FeatureAction, OptionType, and any future addition) is
# boxed/unboxed generically from native code via reflection on fromValue(int)/value().
-keep class * implements sebyone.daasiot_android.enums.NativeEnum {
    *;
}

# IDaasApiEvent is implemented by app code and handed to DaasWrapper.create(); the native layer
# looks up each callback by exact name via GetMethodID on whatever concrete class the app passes.
# Keeping the interface's methods forces R8 to keep matching names on every override too.
-keep interface sebyone.daasiot_android.events.IDaasApiEvent {
    *;
}
