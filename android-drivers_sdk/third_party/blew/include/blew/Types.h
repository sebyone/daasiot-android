// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

namespace blew {

/** MAC address on Linux/Android/Windows/ESP32, CoreBluetooth UUID on Apple platforms. */
using Address = std::string;

/** UUID in textual form. Any case and 16/32/128-bit forms are accepted on input;
 *  the library always returns lowercase 128-bit strings. */
using Uuid = std::string;

/** Byte buffer. It *is* a std::vector<uint8_t>, so it can be passed wherever a vector is expected. */
class Bytes : public std::vector<uint8_t> {
  public:
    using std::vector<uint8_t>::vector;

    Bytes() = default;
    Bytes(const std::vector<uint8_t>& v) : std::vector<uint8_t>(v) {}
    Bytes(std::vector<uint8_t>&& v) noexcept : std::vector<uint8_t>(std::move(v)) {}
    Bytes(const uint8_t* ptr, size_t size) : std::vector<uint8_t>(ptr, ptr + size) {}
    explicit Bytes(const std::string& s) : std::vector<uint8_t>(s.begin(), s.end()) {}

    static Bytes from_hex(const std::string& hex) {
        Bytes out;
        int hi = -1;
        for (char c : hex) {
            int v;
            if (c >= '0' && c <= '9') v = c - '0';
            else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
            else continue;  // separators (spaces, ':', '-') are ignored
            if (hi < 0) {
                hi = v;
            } else {
                out.push_back(static_cast<uint8_t>((hi << 4) | v));
                hi = -1;
            }
        }
        return out;
    }

    std::string to_hex(bool spacing = false) const {
        static const char* digits = "0123456789abcdef";
        std::string out;
        out.reserve(size() * 3);
        for (size_t i = 0; i < size(); i++) {
            if (spacing && i) out.push_back(' ');
            out.push_back(digits[(*this)[i] >> 4]);
            out.push_back(digits[(*this)[i] & 0x0f]);
        }
        return out;
    }

    Bytes slice(size_t start, size_t end) const {
        if (start > size()) start = size();
        if (end > size()) end = size();
        if (end < start) end = start;
        return Bytes(begin() + start, begin() + end);
    }

    std::string to_string() const { return std::string(begin(), end()); }
};

enum class OperatingSystem { WINDOWS, MACOS, IOS, LINUX, ANDROID_OS, ESP32 };

enum AddressType : int32_t { PUBLIC = 0, RANDOM = 1, UNSPECIFIED = 2 };

/** Properties of a characteristic published by a GattServer. */
enum CharacteristicFlags : uint32_t {
    CHR_READ = 1u << 0,
    CHR_WRITE = 1u << 1,              // write with response
    CHR_WRITE_NO_RESPONSE = 1u << 2,  // write command
    CHR_NOTIFY = 1u << 3,
    CHR_INDICATE = 1u << 4,
};

}  // namespace blew
