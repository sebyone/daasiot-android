/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: ConsoleHeader.kt
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
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import sebyone.daasiot_android.demo.DemoViewModel
import sebyone.daasiot_android.demo.ui.theme.ConsoleDotOff
import sebyone.daasiot_android.demo.ui.theme.ConsoleDotOn
import sebyone.daasiot_android.demo.ui.theme.ConsoleMuted
import sebyone.daasiot_android.demo.ui.theme.ConsolePanelAlt

/**
 * Top bar mirroring daasiot_cordova_plugin's demo app header: logo, a SID/DIN badge once an
 * instance is active, a version badge once [DemoViewModel.lastKnownVersion] is known (via
 * "getVersion"), and a colored dot for the native instance status.
 */
@Composable
fun ConsoleHeader(viewModel: DemoViewModel, modifier: Modifier = Modifier) {
    val active = viewModel.instancePtr != null
    Surface(modifier = modifier.fillMaxWidth(), color = MaterialTheme.colorScheme.surface) {
        Row(
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 12.dp),
            verticalAlignment = androidx.compose.ui.Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Text(
                text = "DaasIoT",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.weight(1f),
            )
            if (active) NodeBadge(sid = viewModel.targetSid, din = viewModel.targetDin)
            viewModel.lastKnownVersion?.let { VersionBadge(it) }
            StatusDot(active = active)
        }
    }
}

@Composable
private fun NodeBadge(sid: String, din: String) {
    Row(horizontalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("SID", style = MaterialTheme.typography.labelSmall, color = ConsoleMuted)
        Text(sid, style = MaterialTheme.typography.labelSmall)
        Text("DIN", style = MaterialTheme.typography.labelSmall, color = ConsoleMuted)
        Text(din, style = MaterialTheme.typography.labelSmall)
    }
}

@Composable
private fun StatusDot(active: Boolean) {
    Box(
        modifier = Modifier
            .size(10.dp)
            .background(if (active) ConsoleDotOn else ConsoleDotOff, CircleShape),
    )
}

@Composable
private fun VersionBadge(version: String) {
    Text(
        text = "v$version",
        style = MaterialTheme.typography.labelSmall,
        fontFamily = FontFamily.Monospace,
        color = ConsoleMuted,
        modifier = Modifier
            .background(ConsolePanelAlt, RoundedCornerShape(50))
            .padding(horizontal = 10.dp, vertical = 4.dp),
    )
}
