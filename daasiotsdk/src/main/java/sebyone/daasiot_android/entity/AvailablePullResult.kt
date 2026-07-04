package sebyone.daasiot_android.entity

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.DaasError

/** Result of [DaasWrapper.availablesPull]: a daas_error_t code plus the available DDO count. */
data class AvailablePullResult(val error: DaasError, val count: Long)
