package sebyone.daasiot_android.entity

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.DaasError

/** Result of [DaasWrapper.pull]: a daas_error_t code plus the pulled DDO (null on failure). */
data class PullResult(val error: DaasError, val ddo: DDO?)
