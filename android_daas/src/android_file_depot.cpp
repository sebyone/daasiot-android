/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_file_depot.cpp
 *
 * Implementation of daas::persistence::file_depot and of the file helpers.
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
 * 08/11/2026 - plogiacco@sebyone.it - creation (persistence/file_depot.cpp)
 * 08/11/2026 - plogiacco@sebyone.it - part of android_daas, renamed gnu_file_depot.cpp
 * 09/11/2026 - plogiacco@sebyone.it - Windows (windows-x86_64-mingw32): android_portable.hpp, %LOCALAPPDATA%
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 */

#include "android_file_depot.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <dirent.h>
#include <unistd.h>

#include "android_portable.hpp"

namespace daas { namespace persistence {

namespace {

using portable::isDirectory;
using portable::syncDirectory;

std::string parentOf(const std::string& path) {
    const auto slash = path.rfind('/');
    if (slash == std::string::npos) return ".";
    return slash == 0 ? "/" : path.substr(0, slash);
}

} // namespace

std::string stateDirectory(const std::string& filesDir, const std::string& app) {
    // Android: no HOME nor XDG for an app; its private folder comes from the Java side (Context.getFilesDir()).
    if (filesDir.empty()) return "./" + app + "-state";
    return (filesDir.back() == '/' ? filesDir : filesDir + "/") + app;
}

bool makeDirectories(const std::string& path) {
    if (path.empty() || isDirectory(path)) return !path.empty();
    for (size_t pos = 1; pos != std::string::npos;) {
        pos = path.find('/', pos + 1);
        const std::string part = path.substr(0, pos);
        if (part.empty() || part.back() == ':') continue;          // "C:" of a Windows path
        if (portable::makeDirectory(part.c_str(), ANDROID_DAAS_STATE_DIR_MODE) != 0 && errno != EEXIST) return false;
    }
    return isDirectory(path);
}

bool writeFileAtomic(const std::string& path, const void* data, size_t size) {
    const std::string temporary = path + ".tmp";
    const int fd = ::open(temporary.c_str(), O_WRONLY | O_CREAT | O_TRUNC | portable::kOpenFlags, ANDROID_DAAS_STATE_FILE_MODE);
    if (fd < 0) return false;
    const auto* bytes = static_cast<const unsigned char*>(data);
    size_t done = 0;
    while (done < size) {
        const ssize_t n = ::write(fd, bytes + done, size - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { ::close(fd); ::unlink(temporary.c_str()); return false; }
        done += static_cast<size_t>(n);
    }
    if (portable::syncFile(fd) != 0) { ::close(fd); ::unlink(temporary.c_str()); return false; }
    ::close(fd);
    if (!portable::replaceFile(temporary, path)) { ::unlink(temporary.c_str()); return false; }
    syncDirectory(parentOf(path));
    return true;
}

file_depot::file_depot(std::string directory) : directory_(std::move(directory)) {
    while (directory_.size() > 1 && directory_.back() == '/') directory_.pop_back();
}

file_depot::~file_depot() { close(); }

std::string file_depot::pathOf(unsigned oKey) const { return directory_ + "/k" + std::to_string(oKey) + ".rec"; }

bool file_depot::open(bool read_mode) {
    if (read_mode) {
        struct stat st;
        // Nothing to load (never saved, or cleared): fail, so that loadConfiguration returns false.
        if (::stat(pathOf(0).c_str(), &st) != 0 || st.st_size == 0) { open_ = false; return false; }
        open_ = true; writable_ = false;
        return true;
    }
    if (!makeDirectories(directory_)) return false;
    open_ = true; writable_ = true;
    return true;
}

bool file_depot::close() {
    if (open_ && writable_) syncDirectory(directory_);
    open_ = false; writable_ = false;
    return true;
}

bool file_depot::clearSpace() {
    DIR* dir = ::opendir(directory_.c_str());
    if (dir == nullptr) return errno == ENOENT;
    bool ok = true;
    while (const struct dirent* entry = ::readdir(dir)) {
        const std::string name = entry->d_name;
        auto endsWith = [&name](const char* tail) {
            const size_t n = std::strlen(tail);
            return name.size() > n && name.compare(name.size() - n, n, tail) == 0;
        };
        const bool record = name[0] == 'k' && (endsWith(".rec") || endsWith(".rec.tmp"));
        if (record && ::unlink((directory_ + "/" + name).c_str()) != 0 && errno != ENOENT) ok = false;
    }
    ::closedir(dir);
    syncDirectory(directory_);
    return ok;
}

bool file_depot::getSpaceInfo() {
    const std::string where = isDirectory(directory_) ? directory_ : parentOf(directory_);
    return portable::diskSpace(where, freeBytes_, totalBytes_);
}

bool file_depot::trash(unsigned oKey) {
    return ::unlink(pathOf(oKey).c_str()) == 0 || errno == ENOENT;
}

unsigned file_depot::save(unsigned oKey, unsigned char* pdata, unsigned dsize) {
    if (!makeDirectories(directory_)) return 0;
    return writeFileAtomic(pathOf(oKey), pdata, dsize) ? dsize : 0;
}

unsigned file_depot::load(unsigned oKey, unsigned char* pdata, unsigned dsize) {
    if (pdata == nullptr || dsize == 0) return 0;   // libdaas <= 0.24.6 asked to load a TX payload into nullptr
    const int fd = ::open(pathOf(oKey).c_str(), O_RDONLY | portable::kOpenFlags);
    if (fd < 0) return 0;
    struct stat st;
    if (::fstat(fd, &st) != 0 || static_cast<uint64_t>(st.st_size) > dsize) { ::close(fd); return 0; }
    size_t done = 0;
    const size_t size = static_cast<size_t>(st.st_size);
    while (done < size) {
        const ssize_t n = ::read(fd, pdata + done, size - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        done += static_cast<size_t>(n);
    }
    ::close(fd);
    return done == size ? static_cast<unsigned>(size) : 0;
}

bool file_depot::hasConfiguration() {
    if (!open(true)) return false;
    close();
    return true;
}

}} // namespace daas::persistence
