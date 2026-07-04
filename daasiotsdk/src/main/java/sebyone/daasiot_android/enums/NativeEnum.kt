package sebyone.daasiot_android.enums

/** Implemented by every Kotlin enum that mirrors a native (C-style) enum, so the JNI layer can
 * marshal them generically via reflection (`fromValue(Int)` / `value()`). */
interface NativeEnum {
    fun value(): Int
}
