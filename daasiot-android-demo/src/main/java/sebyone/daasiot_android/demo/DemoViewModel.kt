/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DemoViewModel.kt
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Disclaimer of Warrant
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
 * expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
 * free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
 * Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
 * servicing, repair, or correction.
 * This disclaimer of warranty constitutes an essential part of this License.  No use of any Covered Software
 * is authorized under this License except under this disclaimer.
 *
 * Limitation of Liability
 * Under no circumstances and under no legal theory, whether tort (including negligence), contract, or otherwise,
 * shall any Contributor, or anyone who distributes Covered Software as permitted above, be liable to You for
 * any direct, indirect, special, incidental, or consequential damages of any character including, without
 * limitation, damages for lost profits, loss of goodwill, work stoppage, computer failure or malfunction,
 * or any and all other commercial damages or losses, even if such party shall have been informed of the
 * possibility of such damages.  This limitation of liability shall not apply to liability for death or personal
 * injury resulting from such party's negligence to the extent applicable law prohibits such limitation.
 * Some jurisdictions do not allow the exclusion or limitation of incidental or consequential damages, so this
 * exclusion and limitation may not apply to You.
 *
 * Contributors:
 * s.meduri@sebyone.it - demo app implementation
 *
 */

package sebyone.daasiot_android.demo

import android.os.Handler
import android.os.Looper
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.enums.DaasError
import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.PerformMode
import sebyone.daasiot_android.enums.StreamType
import sebyone.daasiot_android.events.IDaasApiEvent
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/** Log categories, mirroring daasiot_cordova_plugin's demo app (`log(msg, level)` in app.js). */
enum class LogLevel { INFO, SUCCESS, WARN, ERROR, EVENT }

/** One line in the session log. */
data class LogEntry(val timestamp: String, val text: String, val level: LogLevel)

private val daasErrorWireNames = DaasError.entries.map { it.name }.toSet()

/**
 * Holds the session state shared across every [sebyone.daasiot_android.demo.actions.ActionSpec]:
 * the native instance handle, the current [DDO] workspace, the target SID/DIN, and the shared
 * event/result log. Kept in a [ViewModel] so the native instance survives configuration changes
 * instead of leaking (or crashing) on rotation.
 */
class DemoViewModel : ViewModel() {

    var instancePtr by mutableStateOf<Long?>(null)
        private set

    var currentDdo by mutableStateOf<DDO?>(null)

    var targetSid by mutableStateOf("0")
    var targetDin by mutableStateOf("0")

    var autoPerformEnabled by mutableStateOf(false)
        private set

    /** Populated by the "getVersion" action; shown as a badge in the console header. */
    var lastKnownVersion by mutableStateOf<String?>(null)

    val logLines = mutableStateListOf<LogEntry>()

    /** Which tab is selected (persisted here so it survives rotation) and whether it's "Log". */
    var selectedTabIndex by mutableStateOf(0)
        private set
    private var logTabActive = false

    /** Unread log lines since "Log" was last selected - shown as a badge on its tab. */
    var unreadLogCount by mutableStateOf(0)
        private set

    fun selectTab(index: Int, isLogTab: Boolean) {
        selectedTabIndex = index
        logTabActive = isLogTab
        if (isLogTab) unreadLogCount = 0
    }

    private var performLoopJob: Job? = null
    private val mainHandler = Handler(Looper.getMainLooper())
    private val timeFormat = SimpleDateFormat("HH:mm:ss.SSS", Locale.US)

    private val eventHandler = object : IDaasApiEvent {
        override fun dinAccepted(din: Long) = logEvent("dinAccepted(din=$din)")
        override fun ddoReceived(payloadSize: Int, typeset: Int, din: Long) =
            logEvent("ddoReceived(payloadSize=$payloadSize, typeset=$typeset, din=$din)")
        override fun frisbeeReceived(din: Long) = logEvent("frisbeeReceived(din=$din)")
        override fun nodeStateReceived(din: Long) = logEvent("nodeStateReceived(din=$din)")
        override fun atsSyncCompleted(din: Long) = logEvent("atsSyncCompleted(din=$din)")
        override fun frisbeeDperfCompleted(din: Long, packetsSent: Long, blockSize: Long) =
            logEvent("frisbeeDperfCompleted(din=$din, packetsSent=$packetsSent, blockSize=$blockSize)")
        override fun networkDiscovered(din: Long, sid: Long, link: LinkType) =
            logEvent("networkDiscovered(din=$din, sid=$sid, link=$link)")
        override fun nodeConnectedToNetwork(sid: Long, din: Long) =
            logEvent("nodeConnectedToNetwork(sid=$sid, din=$din)")
        override fun streamInfoReceived(din: Long, packetType: StreamType, streamId: Long) =
            logEvent("streamInfoReceived(din=$din, packetType=$packetType, streamId=$streamId)")
    }

    fun targetSidOrDefault(): Long = targetSid.toLongOrNull() ?: 0L
    fun targetDinOrDefault(): Long = targetDin.toLongOrNull() ?: 0L

    fun log(line: String, level: LogLevel = LogLevel.INFO) {
        val entry = LogEntry(timeFormat.format(Date()), line, level)
        mainHandler.post {
            logLines.add(0, entry)
            if (!logTabActive) unreadLogCount++
        }
    }

    private fun logEvent(line: String) = log("[EVENT] $line", LogLevel.EVENT)

    /** A [DaasError] result other than `ERROR_NONE` is a [LogLevel.WARN], not a hard failure. */
    fun classify(result: String): LogLevel =
        if (result in daasErrorWireNames && result != DaasError.ERROR_NONE.name) LogLevel.WARN else LogLevel.SUCCESS

    fun clearLog() {
        logLines.clear()
    }

    fun createInstance(lhver: String) {
        val ptr = DaasWrapper.create(eventHandler, lhver.ifBlank { null })
        instancePtr = ptr
        log("[create] -> ptr=$ptr", LogLevel.SUCCESS)
    }

    fun destroyInstance() {
        val ptr = instancePtr ?: return
        setAutoPerform(false, PerformMode.PERFORM_CORE_NO_THREAD)
        currentDdo?.close()
        currentDdo = null
        DaasWrapper.destroy(ptr)
        instancePtr = null
        log("[destroy] -> ok", LogLevel.WARN)
    }

    /** Starts/stops the background loop `doPerform` needs in [PerformMode.PERFORM_CORE_NO_THREAD]. */
    fun setAutoPerform(enabled: Boolean, mode: PerformMode) {
        performLoopJob?.cancel()
        performLoopJob = null
        autoPerformEnabled = false
        if (!enabled) return
        val ptr = instancePtr ?: return
        autoPerformEnabled = true
        performLoopJob = viewModelScope.launch {
            while (isActive) {
                DaasWrapper.doPerform(ptr, mode)
                delay(100)
            }
        }
    }

    override fun onCleared() {
        performLoopJob?.cancel()
        currentDdo?.close()
        instancePtr?.let { DaasWrapper.destroy(it) }
        instancePtr = null
        super.onCleared()
    }
}
