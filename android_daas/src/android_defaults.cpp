/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_defaults.cpp
 *
 * The hardware version callback of daas::sdk and its default implementation (device tree, uname).
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
 * plogiacco@sebyone.it - design, implementation and documentation
 *
 * Changelog:
 * 08/11/2026 - plogiacco@sebyone.it - creation
 * 09/11/2026 - plogiacco@sebyone.it - Windows (windows-x86_64-mingw32): "WIN-<arch>"
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 */

#include "android_defaults.hpp"

#include <fstream>
#include <mutex>

#include <sys/utsname.h>

namespace daas { namespace sdk {

namespace {

std::mutex providerMutex;
HardwareVersionProvider provider;

const char* systemName() { return "AND"; }


const char* architecture() {
    struct utsname name;
    if (::uname(&name) != 0) return "";
    const std::string machine = name.machine;
    if (machine == "x86_64" || machine == "amd64") return "-X64";
    if (machine == "aarch64" || machine == "arm64") return "-A64";
    if (machine.rfind("arm", 0) == 0) return "-A32";
    if (machine.size() == 4 && machine[0] == 'i' && machine.compare(2, 2, "86") == 0) return "-X32";
    if (machine == "riscv64") return "-RV64";
    if (machine.rfind("mips", 0) == 0) return "-MIPS";
    return "";
}

} // namespace

void setHardwareVersionProvider(HardwareVersionProvider callback) {
    std::lock_guard<std::mutex> lock(providerMutex);
    provider = std::move(callback);
}

std::string detectHardware() {
    return std::string(systemName()) + architecture();
}

std::string hardwareVersion() {
    HardwareVersionProvider callback;
    {
        std::lock_guard<std::mutex> lock(providerMutex);
        callback = provider;
    }
    std::string version = callback ? callback() : detectHardware();
    if (version.empty()) version = detectHardware();
    return version.substr(0, kHardwareVersionMax);
}

}} // namespace daas::sdk
