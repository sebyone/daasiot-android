/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: LogPanel.kt
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

package sebyone.daasiot_android.demo.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import sebyone.daasiot_android.demo.DemoViewModel
import sebyone.daasiot_android.demo.LogEntry
import sebyone.daasiot_android.demo.LogLevel
import sebyone.daasiot_android.demo.ui.theme.ConsoleAccentDark
import sebyone.daasiot_android.demo.ui.theme.ConsoleErr
import sebyone.daasiot_android.demo.ui.theme.ConsoleMuted
import sebyone.daasiot_android.demo.ui.theme.ConsoleOk
import sebyone.daasiot_android.demo.ui.theme.ConsolePanelAlt
import sebyone.daasiot_android.demo.ui.theme.ConsoleWarn

/** The "Log" tab's content: a toolbar (clear button) plus a scrollable, newest-first log. */
@Composable
fun LogPanel(viewModel: DemoViewModel, modifier: Modifier = Modifier) {
    Column(modifier = modifier.fillMaxSize().padding(horizontal = 12.dp, vertical = 4.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.End,
        ) {
            TextButton(onClick = { viewModel.clearLog() }) { Text("Pulisci") }
        }
        LazyColumn(modifier = Modifier.fillMaxSize()) {
            items(viewModel.logLines) { entry -> LogLine(entry) }
        }
    }
}

private fun colorFor(level: LogLevel): androidx.compose.ui.graphics.Color = when (level) {
    LogLevel.SUCCESS -> ConsoleOk
    LogLevel.WARN -> ConsoleWarn
    LogLevel.ERROR -> ConsoleErr
    LogLevel.EVENT -> ConsoleAccentDark
    LogLevel.INFO -> ConsoleMuted
}

private fun pillFor(level: LogLevel): String = when (level) {
    LogLevel.SUCCESS -> "OK"
    LogLevel.WARN -> "WARN"
    LogLevel.ERROR -> "ERR"
    LogLevel.EVENT -> "EVT"
    LogLevel.INFO -> "INFO"
}

@Composable
private fun LogLine(entry: LogEntry) {
    Row(
        modifier = Modifier.fillMaxWidth().padding(vertical = 2.dp),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
    ) {
        StatusPill(level = entry.level)
        Text(
            text = entry.timestamp,
            style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace,
            color = ConsoleMuted,
        )
        Text(
            text = entry.text,
            style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace,
            color = if (entry.level == LogLevel.INFO) MaterialTheme.colorScheme.onSurface else colorFor(entry.level),
            modifier = Modifier.weight(1f),
        )
    }
}

@Composable
private fun StatusPill(level: LogLevel) {
    Text(
        text = pillFor(level),
        style = MaterialTheme.typography.labelSmall,
        fontFamily = FontFamily.Monospace,
        color = colorFor(level),
        modifier = Modifier
            .background(ConsolePanelAlt, RoundedCornerShape(50))
            .padding(horizontal = 6.dp, vertical = 1.dp),
    )
}
