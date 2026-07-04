/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: ActionRow.kt
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

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import sebyone.daasiot_android.demo.DemoViewModel
import sebyone.daasiot_android.demo.LogLevel
import sebyone.daasiot_android.demo.actions.ActionSpec
import sebyone.daasiot_android.demo.actions.ParamSpec

/**
 * Generic renderer for one [ActionSpec]: one input control per [ParamSpec], plus an "Esegui"
 * button that collects the current values and invokes the action, logging the result/error.
 */
@Composable
fun ActionRow(action: ActionSpec, viewModel: DemoViewModel, modifier: Modifier = Modifier) {
    val fieldStates = remember(action.title) {
        action.params.map { spec ->
            mutableStateOf<Any?>(
                when (spec) {
                    is ParamSpec.LongParam -> spec.default.toString()
                    is ParamSpec.IntParam -> spec.default.toString()
                    is ParamSpec.TextParam -> spec.default
                    is ParamSpec.BytesParam -> spec.default
                    is ParamSpec.BoolParam -> spec.default
                    is ParamSpec.EnumParam<*> -> spec.default
                },
            )
        }
    }

    val enabled = !action.requiresInstance || viewModel.instancePtr != null

    Card(modifier = modifier.fillMaxWidth().padding(horizontal = 12.dp, vertical = 4.dp)) {
        Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            Text(action.title, style = MaterialTheme.typography.titleSmall)
            action.description?.let { Text(it, style = MaterialTheme.typography.bodySmall) }

            action.params.forEachIndexed { index, spec ->
                val state = fieldStates[index]
                ParamInput(spec = spec, state = state)
            }

            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(enabled = enabled, onClick = {
                    val values = action.params.mapIndexed { index, spec ->
                        val raw = fieldStates[index].value
                        when (spec) {
                            is ParamSpec.LongParam -> (raw as String).toLongOrNull() ?: spec.default
                            is ParamSpec.IntParam -> (raw as String).toIntOrNull() ?: spec.default
                            is ParamSpec.TextParam, is ParamSpec.BytesParam -> raw
                            is ParamSpec.BoolParam -> raw
                            is ParamSpec.EnumParam<*> -> raw
                        }
                    }
                    try {
                        val result = action.invoke(viewModel, values)
                        viewModel.log("[${action.title}] -> $result", viewModel.classify(result))
                    } catch (e: Exception) {
                        viewModel.log("[${action.title}] ERRORE: ${e.message}", LogLevel.ERROR)
                    }
                }) {
                    Text("Esegui")
                }
                if (!enabled) {
                    Text(
                        "Richiede un'istanza attiva",
                        style = MaterialTheme.typography.labelSmall,
                        modifier = Modifier.padding(top = 12.dp),
                    )
                }
            }
        }
    }
}

@Composable
private fun ParamInput(spec: ParamSpec, state: androidx.compose.runtime.MutableState<Any?>) {
    when (spec) {
        is ParamSpec.LongParam, is ParamSpec.IntParam, is ParamSpec.TextParam, is ParamSpec.BytesParam ->
            OutlinedTextField(
                value = state.value as String,
                onValueChange = { state.value = it },
                label = { Text(spec.label) },
                singleLine = spec !is ParamSpec.BytesParam,
                modifier = Modifier.fillMaxWidth(),
            )

        is ParamSpec.BoolParam -> Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
        ) {
            Text(spec.label)
            Switch(checked = state.value as Boolean, onCheckedChange = { state.value = it })
        }

        is ParamSpec.EnumParam<*> -> EnumDropdown(spec = spec, state = state)
    }
}

@Composable
private fun EnumDropdown(spec: ParamSpec.EnumParam<*>, state: androidx.compose.runtime.MutableState<Any?>) {
    var expanded by remember { mutableStateOf(false) }
    Box {
        OutlinedButton(onClick = { expanded = true }, modifier = Modifier.fillMaxWidth()) {
            Text("${spec.label}: ${(state.value as Enum<*>).name}")
        }
        DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }) {
            spec.options.forEach { option ->
                val enumOption = option as Enum<*>
                DropdownMenuItem(
                    text = { Text(enumOption.name) },
                    onClick = {
                        state.value = enumOption
                        expanded = false
                    },
                )
            }
        }
    }
}
