/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: ActionCatalog.kt
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

package sebyone.daasiot_android.demo.actions

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.demo.DemoViewModel
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.entity.FeatureRequest
import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DaasError
import sebyone.daasiot_android.enums.DiscoveryState
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.FeatureAction
import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.OptionType
import sebyone.daasiot_android.enums.PerformMode

private fun requireDdo(vm: DemoViewModel): DDO =
    vm.currentDdo ?: error("Crea prima un DDO nel workspace")

/**
 * Every [DaasWrapper] function/overload, plus the [DDO] instance methods needed to exercise
 * [DaasWrapper.push]/[DaasWrapper.pull] meaningfully. SID/DIN are read from [DemoViewModel]'s
 * shared session fields rather than duplicated as per-row inputs.
 */
object ActionCatalog {

    fun categories(): List<ActionCategory> = listOf(
        setupCategory(),
        networkCategory(),
        ddoCategory(),
        realtimeCategory(),
        featureCategory(),
    )

    private fun setupCategory() = ActionCategory(
        "Setup",
        listOf(
            ActionSpec(
                "create",
                "Crea una nuova istanza nativa DaaS.",
                params = listOf(ParamSpec.TextParam("lhver (opzionale)")),
                requiresInstance = false,
            ) { vm, values ->
                vm.createInstance(values[0] as String)
                "ptr=${vm.instancePtr}"
            },
            ActionSpec("destroy", "Rilascia l'istanza nativa corrente.") { vm, _ ->
                vm.destroyInstance()
                "ok"
            },
            ActionSpec("getVersion") { vm, _ ->
                DaasWrapper.getVersion(vm.instancePtr!!).also { vm.lastKnownVersion = it }
            },
            ActionSpec("getBuildInfo") { vm, _ -> DaasWrapper.getBuildInfo(vm.instancePtr!!) },
            ActionSpec("listAvailableDrivers") { vm, _ -> DaasWrapper.listAvailableDrivers(vm.instancePtr!!) },
            ActionSpec("doInit", "Usa SID/DIN della sessione.") { vm, _ ->
                DaasWrapper.doInit(vm.instancePtr!!, vm.targetSidOrDefault(), vm.targetDinOrDefault()).toString()
            },
            ActionSpec("doEnd") { vm, _ -> DaasWrapper.doEnd(vm.instancePtr!!).toString() },
            ActionSpec("reset") { vm, _ -> DaasWrapper.reset(vm.instancePtr!!).toString() },
            ActionSpec(
                "doPerform",
                "Con \"loop\" attivo, resta in esecuzione ciclica ogni 100ms finché non premi di nuovo Esegui con loop disattivato (richiesto in modalità PERFORM_CORE_NO_THREAD).",
                params = listOf(
                    ParamSpec.EnumParam("Modalità", PerformMode.entries, PerformMode.PERFORM_CORE_NO_THREAD),
                    ParamSpec.BoolParam("Loop 100ms"),
                ),
            ) { vm, values ->
                val mode = values[0] as PerformMode
                val loop = values[1] as Boolean
                vm.setAutoPerform(loop, mode)
                if (loop) "loop avviato (mode=$mode)" else DaasWrapper.doPerform(vm.instancePtr!!, mode).toString()
            },
            ActionSpec(
                "enableDriver",
                params = listOf(
                    ParamSpec.EnumParam("Link", LinkType.entries, LinkType.LINK_INET4),
                    ParamSpec.TextParam("URI locale"),
                ),
            ) { vm, values ->
                DaasWrapper.enableDriver(vm.instancePtr!!, values[0] as LinkType, values[1] as String).toString()
            },
            ActionSpec("getStatus") { vm, _ -> formatNodeInfo(DaasWrapper.getStatus(vm.instancePtr!!)) },
            ActionSpec(
                "setAcceptRequestsLevel",
                params = listOf(ParamSpec.EnumParam("Policy", AcceptRequestPolicy.entries, AcceptRequestPolicy.TRUST_ALL)),
            ) { vm, values ->
                DaasWrapper.setAcceptRequestsLevel(vm.instancePtr!!, values[0] as AcceptRequestPolicy)
                "ok"
            },
            ActionSpec("storeConfiguration", params = listOf(ParamSpec.LongParam("depotPtr"))) { vm, values ->
                DaasWrapper.storeConfiguration(vm.instancePtr!!, values[0] as Long).toString()
            },
            ActionSpec("loadConfiguration", params = listOf(ParamSpec.LongParam("depotPtr"))) { vm, values ->
                DaasWrapper.loadConfiguration(vm.instancePtr!!, values[0] as Long).toString()
            },
            ActionSpec("doStatisticsReset") { vm, _ -> DaasWrapper.doStatisticsReset(vm.instancePtr!!).toString() },
            ActionSpec("getSystemStatistics", params = listOf(ParamSpec.IntParam("code"))) { vm, values ->
                DaasWrapper.getSystemStatistics(vm.instancePtr!!, values[0] as Int).toString()
            },
        ),
    )

    private fun networkCategory() = ActionCategory(
        "Network",
        listOf(
            ActionSpec(
                "map",
                "Usa il DIN della sessione.",
                params = listOf(
                    ParamSpec.EnumParam("Link", LinkType.entries, LinkType.LINK_NONE),
                    ParamSpec.TextParam("URI (opzionale)"),
                    ParamSpec.TextParam("skey (opzionale)"),
                ),
            ) { vm, values ->
                val uri = (values[1] as String).ifBlank { null }
                val skey = (values[2] as String).ifBlank { null }
                DaasWrapper.map(vm.instancePtr!!, vm.targetDinOrDefault(), values[0] as LinkType, uri, skey).toString()
            },
            ActionSpec("remove", "Usa il DIN della sessione.") { vm, _ ->
                DaasWrapper.remove(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec(
                "discovery (per link)",
                params = listOf(ParamSpec.EnumParam("Link", LinkType.entries, LinkType.LINK_NONE)),
            ) { vm, values ->
                DaasWrapper.discovery(vm.instancePtr!!, values[0] as LinkType).toString()
            },
            ActionSpec("discovery (per SID)", "Usa il SID della sessione.") { vm, _ ->
                DaasWrapper.discovery(vm.instancePtr!!, vm.targetSidOrDefault()).toString()
            },
            ActionSpec(
                "join",
                "Usa il SID della sessione. Chiamata bloccante.",
                params = listOf(
                    ParamSpec.EnumParam("Link", LinkType.entries, LinkType.LINK_NONE),
                    ParamSpec.LongParam("timeoutMillis", 5000),
                ),
            ) { vm, values ->
                DaasWrapper.join(vm.instancePtr!!, vm.targetSidOrDefault(), values[0] as LinkType, values[1] as Long).toString()
            },
            ActionSpec(
                "setDiscoveryState",
                params = listOf(ParamSpec.EnumParam("Stato", DiscoveryState.entries, DiscoveryState.OFF)),
            ) { vm, values ->
                DaasWrapper.setDiscoveryState(vm.instancePtr!!, values[0] as DiscoveryState)
                "ok"
            },
            ActionSpec("listNodes") { vm, _ ->
                DaasWrapper.listNodes(vm.instancePtr!!).joinToString("\n") { "sid=${it.sid} din=${it.din}" }
                    .ifBlank { "(vuoto)" }
            },
            ActionSpec("getNodeList") { vm, _ ->
                DaasWrapper.getNodeList(vm.instancePtr!!).joinToString("\n") {
                    "din=${it.din} owner=${it.channelOwner} link=${it.link} uri=${it.uri} direct=${it.directChannel}"
                }.ifBlank { "(vuoto)" }
            },
            ActionSpec("getAllNodes", "Usa il SID della sessione.") { vm, _ ->
                DaasWrapper.getAllNodes(vm.instancePtr!!, vm.targetSidOrDefault()).joinToString().ifBlank { "(vuoto)" }
            },
            ActionSpec("locate (per DIN)", "Usa il DIN della sessione. Chiamata bloccante.") { vm, _ ->
                DaasWrapper.locate(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec(
                "locate (per feature)",
                "Chiamata bloccante.",
                params = listOf(
                    ParamSpec.EnumParam("Feature", Feature.entries, Feature.STORAGE),
                    ParamSpec.IntParam("timeoutMillis", 1000),
                    ParamSpec.IntParam("ttl", 10),
                ),
            ) { vm, values ->
                DaasWrapper.locate(vm.instancePtr!!, values[0] as Feature, values[1] as Int, values[2] as Int).toString()
            },
            ActionSpec("createNetwork") { vm, _ -> DaasWrapper.createNetwork(vm.instancePtr!!).toString() },
            ActionSpec("unbindNetwork") { vm, _ -> DaasWrapper.unbindNetwork(vm.instancePtr!!).toString() },
            ActionSpec(
                "setDDOPolicy",
                params = listOf(ParamSpec.EnumParam("Policy", DDOPolicy.entries, DDOPolicy.SKIP_ON_FAILURE)),
            ) { vm, values ->
                DaasWrapper.setDDOPolicy(vm.instancePtr!!, values[0] as DDOPolicy).toString()
            },
            ActionSpec(
                "setOptions",
                params = listOf(
                    ParamSpec.EnumParam("Opzione", OptionType.entries, OptionType.SET_DDO_RX_BUFFER_SIZE),
                    ParamSpec.LongParam("value"),
                ),
            ) { vm, values ->
                DaasWrapper.setOptions(vm.instancePtr!!, values[0] as OptionType, values[1] as Long).toString()
            },
            ActionSpec(
                "errorToString",
                params = listOf(ParamSpec.EnumParam("Errore", DaasError.entries, DaasError.ERROR_NONE)),
            ) { vm, values ->
                DaasWrapper.errorToString(vm.instancePtr!!, values[0] as DaasError)
            },
            ActionSpec("sendStatus", "Usa il DIN della sessione.") { vm, _ ->
                DaasWrapper.sendStatus(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec("status", "Usa il DIN della sessione.") { vm, _ ->
                formatNodeInfo(DaasWrapper.status(vm.instancePtr!!, vm.targetDinOrDefault()))
            },
            ActionSpec(
                "fetch",
                "Usa il DIN della sessione.",
                params = listOf(ParamSpec.IntParam("opts")),
            ) { vm, values ->
                formatNodeInfo(DaasWrapper.fetch(vm.instancePtr!!, vm.targetDinOrDefault(), values[0] as Int))
            },
            ActionSpec("getSyncedTimestamp") { vm, _ -> DaasWrapper.getSyncedTimestamp(vm.instancePtr!!).toString() },
            ActionSpec(
                "syncNode",
                "Usa il DIN della sessione.",
                params = listOf(ParamSpec.IntParam("timezone")),
            ) { vm, values ->
                formatNodeInfo(DaasWrapper.syncNode(vm.instancePtr!!, vm.targetDinOrDefault(), values[0] as Int))
            },
            ActionSpec(
                "syncNet",
                "Usa il DIN della sessione.",
                params = listOf(ParamSpec.IntParam("bubbleTime")),
            ) { vm, values ->
                formatNodeInfo(DaasWrapper.syncNet(vm.instancePtr!!, vm.targetDinOrDefault(), values[0] as Int))
            },
        ),
    )

    private fun realtimeCategory() = ActionCategory(
        "Stream",
        listOf(
            ActionSpec("use", "Usa il DIN della sessione. Chiamata bloccante.") { vm, _ ->
                DaasWrapper.use(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec("end", "Usa il DIN della sessione.") { vm, _ ->
                DaasWrapper.end(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec(
                "send",
                "Usa il DIN della sessione.",
                params = listOf(ParamSpec.BytesParam("dati (UTF-8)")),
            ) { vm, values ->
                val sent = DaasWrapper.send(vm.instancePtr!!, vm.targetDinOrDefault(), (values[0] as String).toUtf8Bytes())
                "$sent byte inviati"
            },
            ActionSpec("received", "Usa il DIN della sessione.") { vm, _ ->
                DaasWrapper.received(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec("receive", "Usa il DIN della sessione.") { vm, _ ->
                DaasWrapper.receive(vm.instancePtr!!, vm.targetDinOrDefault()).toUtf8Text()
            },
        ),
    )

    private fun ddoCategory() = ActionCategory(
        "DDO",
        listOf(
            ActionSpec("pull", "Usa il DIN della sessione. Il DDO ricevuto sostituisce quello nel workspace.") { vm, _ ->
                val result = DaasWrapper.pull(vm.instancePtr!!, vm.targetDinOrDefault())
                if (result.ddo != null) {
                    vm.currentDdo?.close()
                    vm.currentDdo = result.ddo
                }
                "error=${result.error}, ddo=${if (result.ddo != null) "caricato nel workspace" else "null"}"
            },
            ActionSpec("push", "Usa il DIN della sessione e il DDO del workspace.") { vm, _ ->
                DaasWrapper.push(vm.instancePtr!!, vm.targetDinOrDefault(), requireDdo(vm)).toString()
            },
            ActionSpec("availablesPull", "Usa il DIN della sessione.") { vm, _ ->
                val result = DaasWrapper.availablesPull(vm.instancePtr!!, vm.targetDinOrDefault())
                "error=${result.error}, count=${result.count}"
            },
            ActionSpec("addTypeset", params = listOf(ParamSpec.IntParam("typesetCode"))) { vm, values ->
                DaasWrapper.addTypeset(vm.instancePtr!!, values[0] as Int).toString()
            },
            ActionSpec("frisbee", "Usa il DIN della sessione.") { vm, _ ->
                DaasWrapper.frisbee(vm.instancePtr!!, vm.targetDinOrDefault()).toString()
            },
            ActionSpec(
                "DDO: crea vuoto",
                "Sostituisce il DDO corrente nel workspace.",
                requiresInstance = false,
            ) { vm, _ ->
                vm.currentDdo?.close()
                vm.currentDdo = DDO()
                "creato"
            },
            ActionSpec(
                "DDO: crea con typeset",
                "Sostituisce il DDO corrente nel workspace.",
                params = listOf(ParamSpec.IntParam("typeset")),
                requiresInstance = false,
            ) { vm, values ->
                vm.currentDdo?.close()
                vm.currentDdo = DDO(values[0] as Int)
                "creato"
            },
            ActionSpec(
                "DDO: imposta typeset",
                params = listOf(ParamSpec.IntParam("typeset")),
                requiresInstance = false,
            ) { vm, values ->
                requireDdo(vm).setTypeset(values[0] as Int)
                "ok"
            },
            ActionSpec(
                "DDO: imposta payload",
                params = listOf(ParamSpec.BytesParam("payload (UTF-8)")),
                requiresInstance = false,
            ) { vm, values ->
                "${requireDdo(vm).setPayload((values[0] as String).toUtf8Bytes())} byte scritti"
            },
            ActionSpec(
                "DDO: aggiungi al payload",
                params = listOf(ParamSpec.BytesParam("dati (UTF-8)")),
                requiresInstance = false,
            ) { vm, values ->
                "${requireDdo(vm).appendPayloadData((values[0] as String).toUtf8Bytes())} byte totali"
            },
            ActionSpec(
                "DDO: alloca payload",
                params = listOf(ParamSpec.LongParam("size")),
                requiresInstance = false,
            ) { vm, values ->
                "${requireDdo(vm).allocatePayload(values[0] as Long)} byte allocati"
            },
            ActionSpec("DDO: svuota payload", requiresInstance = false) { vm, _ ->
                requireDdo(vm).clearPayload()
                "ok"
            },
            ActionSpec("DDO: leggi typeset", requiresInstance = false) { vm, _ ->
                requireDdo(vm).getTypeset().toString()
            },
            ActionSpec("DDO: leggi origin", requiresInstance = false) { vm, _ ->
                requireDdo(vm).getOrigin().toString()
            },
            ActionSpec("DDO: leggi timestamp", requiresInstance = false) { vm, _ ->
                requireDdo(vm).getTimestamp().toString()
            },
            ActionSpec("DDO: leggi dimensione payload", requiresInstance = false) { vm, _ ->
                requireDdo(vm).getPayloadSize().toString()
            },
            ActionSpec("DDO: leggi payload", requiresInstance = false) { vm, _ ->
                requireDdo(vm).getPayload().toUtf8Text()
            },
            ActionSpec(
                "DDO: leggi payload binario",
                params = listOf(ParamSpec.IntParam("offset"), ParamSpec.IntParam("maxSize")),
                requiresInstance = false,
            ) { vm, values ->
                requireDdo(vm).getPayloadAsBinary(values[0] as Int, values[1] as Int).toUtf8Text()
            },
            ActionSpec("DDO: chiudi", requiresInstance = false) { vm, _ ->
                vm.currentDdo?.close()
                vm.currentDdo = null
                "chiuso"
            },
        ),
    )

    private fun featureCategory() = ActionCategory(
        "Feature",
        listOf(
            ActionSpec(
                "addNodeFeatures",
                params = listOf(ParamSpec.EnumParam("Feature", Feature.entries, Feature.STORAGE)),
            ) { vm, values ->
                DaasWrapper.addNodeFeatures(vm.instancePtr!!, values[0] as Feature).toString()
            },
            ActionSpec("getNodeFeatures") { vm, _ ->
                DaasWrapper.getNodeFeatures(vm.instancePtr!!).joinToString().ifBlank { "(nessuna)" }
            },
            ActionSpec(
                "nodeHasFeature",
                "Usa il DIN della sessione.",
                params = listOf(ParamSpec.EnumParam("Feature", Feature.entries, Feature.STORAGE)),
            ) { vm, values ->
                DaasWrapper.nodeHasFeature(vm.instancePtr!!, vm.targetDinOrDefault(), values[0] as Feature).toString()
            },
            ActionSpec(
                "requestFeature",
                "Usa il DIN della sessione. Restituisce sempre ERROR_NOT_IMPLEMENTED oggi (vedi doc di DaasWrapper.requestFeature). Max 16 byte per params, 128 per payload.",
                params = listOf(
                    ParamSpec.EnumParam("Feature", Feature.entries, Feature.STORAGE),
                    ParamSpec.EnumParam("Action", FeatureAction.entries, FeatureAction.NONE),
                    ParamSpec.BytesParam("params (UTF-8, max 16 byte)"),
                    ParamSpec.BytesParam("payload (UTF-8, max 128 byte)"),
                    ParamSpec.LongParam("timeoutMillis", 1000),
                ),
            ) { vm, values ->
                val request = FeatureRequest(
                    feature = values[0] as Feature,
                    action = values[1] as FeatureAction,
                    params = (values[2] as String).toUtf8Bytes(),
                    payload = (values[3] as String).toUtf8Bytes(),
                )
                DaasWrapper.requestFeature(vm.instancePtr!!, vm.targetDinOrDefault(), request, values[4] as Long).toString()
            },
        ),
    )
}
