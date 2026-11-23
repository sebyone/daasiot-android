// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <utility>

namespace blew {

/** Outcome of every fallible call. The library never throws; details of a failure are logged. */
enum class Status : int {
    Ok = 0,
    NotInitialized,   // default-constructed handle, or the stack is not started
    BluetoothOff,     // radio missing or powered off
    Unauthorized,     // missing OS permission (Android runtime permissions, macOS/iOS privacy)
    NotConnected,     // the peer is not (or no longer) connected
    NotFound,         // unknown service / characteristic / peer
    NotSupported,     // the operation is not possible on this platform or characteristic
    InvalidArgument,
    Busy,             // another operation is in progress, or the stack refused for now
    NoResources,      // out of memory / connection slots
    Timeout,
    Failed,           // anything else reported by the stack
};

const char* to_string(Status status);

/** A value or the Status explaining why there is none. */
template <typename T>
class Result {
  public:
    Result(Status status) : status_(status) {}  // NOLINT: implicit on purpose
    Result(T value) : status_(Status::Ok), value_(std::move(value)) {}  // NOLINT

    bool ok() const { return status_ == Status::Ok; }
    explicit operator bool() const { return ok(); }
    Status status() const { return status_; }

    const T& value() const& { return value_; }
    T& value() & { return value_; }
    T&& value() && { return std::move(value_); }
    T value_or(T fallback) const { return ok() ? value_ : std::move(fallback); }

  private:
    Status status_;
    T value_{};
};

}  // namespace blew

/** Returns from the enclosing function if `expr` does not evaluate to Status::Ok. */
#define BLEW_TRY(expr)                                     \
    do {                                                   \
        ::blew::Status _blew_s = (expr);                   \
        if (_blew_s != ::blew::Status::Ok) return _blew_s; \
    } while (0)
