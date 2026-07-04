/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DemoScreen.kt
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
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.PrimaryScrollableTabRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import sebyone.daasiot_android.demo.DemoViewModel
import sebyone.daasiot_android.demo.actions.ActionCategory
import sebyone.daasiot_android.demo.actions.ActionCatalog
import sebyone.daasiot_android.demo.ui.theme.ConsoleErr

/**
 * Screen layout mirroring daasiot_cordova_plugin's demo app: header, a tab bar (one tab per
 * workflow area plus "Log"), and per-tab content built from a small identifier card (SID/DIN or
 * a remote DIN, right where it's used - not one always-visible global panel) followed by every
 * action in that category.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DemoScreen(viewModel: DemoViewModel) {
    val categories = remember { ActionCatalog.categories() }
    val logTabIndex = categories.size

    Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background) {
        Column(modifier = Modifier.fillMaxSize()) {
            ConsoleHeader(viewModel = viewModel)

            PrimaryScrollableTabRow(selectedTabIndex = viewModel.selectedTabIndex) {
                categories.forEachIndexed { index, category ->
                    Tab(
                        selected = viewModel.selectedTabIndex == index,
                        onClick = { viewModel.selectTab(index, isLogTab = false) },
                        text = { Text(category.name) },
                    )
                }
                Tab(
                    selected = viewModel.selectedTabIndex == logTabIndex,
                    onClick = { viewModel.selectTab(logTabIndex, isLogTab = true) },
                    text = { LogTabLabel(unreadCount = viewModel.unreadLogCount) },
                )
            }

            if (viewModel.selectedTabIndex == logTabIndex) {
                LogPanel(viewModel = viewModel, modifier = Modifier.weight(1f))
            } else {
                CategoryTabContent(
                    category = categories[viewModel.selectedTabIndex],
                    viewModel = viewModel,
                    modifier = Modifier.weight(1f),
                )
            }
        }
    }
}

@Composable
private fun LogTabLabel(unreadCount: Int) {
    Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(4.dp)) {
        Text("Log")
        if (unreadCount > 0) {
            Text(
                text = if (unreadCount > 99) "99+" else unreadCount.toString(),
                style = MaterialTheme.typography.labelSmall,
                color = Color.White,
                modifier = Modifier
                    .background(ConsoleErr, CircleShape)
                    .padding(horizontal = 5.dp, vertical = 1.dp),
            )
        }
    }
}

@Composable
private fun CategoryTabContent(category: ActionCategory, viewModel: DemoViewModel, modifier: Modifier = Modifier) {
    Column(modifier = modifier.fillMaxSize()) {
        val context = contextFor(category.name)
        val ddoStatus = if (category.name == "DDO") {
            viewModel.currentDdo?.let { "DDO workspace: attivo (typeset=${it.getTypeset()})" } ?: "DDO workspace: vuoto"
        } else {
            null
        }
        TabTargetCard(
            viewModel = viewModel,
            dinLabel = context.dinLabel,
            showSid = context.showSid,
            extraStatus = ddoStatus,
        )
        LazyVerticalGrid(columns = GridCells.Adaptive(minSize = 300.dp), modifier = Modifier.weight(1f)) {
            items(category.actions, key = { it.title }) { action ->
                ActionRow(action = action, viewModel = viewModel)
            }
        }
    }
}

private data class TabContext(val showSid: Boolean, val dinLabel: String)

private fun contextFor(categoryName: String): TabContext = when (categoryName) {
    "Setup" -> TabContext(showSid = true, dinLabel = "DIN")
    "Network" -> TabContext(showSid = true, dinLabel = "DIN remoto")
    else -> TabContext(showSid = false, dinLabel = "DIN remoto")
}
