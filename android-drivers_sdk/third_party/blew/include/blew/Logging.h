// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <functional>
#include <string>

#include <blew/export.h>

namespace blew {
namespace Logging {

enum Level : int { None = 0, Fatal, Error, Warn, Info, Debug, Verbose };

using Callback = std::function<void(Level level, const std::string& module, const std::string& message)>;

/** Messages above `level` are dropped. Default: Info. */
BLEW_EXPORT void set_level(Level level);
BLEW_EXPORT Level get_level();

/** Replace the default sink (stderr, ESP_LOG on ESP32, logcat on Android). nullptr restores it. */
BLEW_EXPORT void set_callback(Callback callback);

}  // namespace Logging
}  // namespace blew
