/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_portable.hpp
 *
 * Private header of android_daas: the few system calls that differ between Android (POSIX, bionic) and Windows (MinGW-w64,
 * target windows-x86_64-mingw32), so that the sources of android_daas stay one implementation. Not installed.
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
 * 09/11/2026 - plogiacco@sebyone.it - creation (port of android_daas to windows-x86_64-mingw32)
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 * 24/11/2026 - l.grillo@sebyone.it - no namespace: the API of android_daas is global, like libdaas
 */

#ifndef _ANDROID_PORTABLE_H__
#define _ANDROID_PORTABLE_H__

#include <cstdint>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>

#include <sys/statvfs.h>
#include <unistd.h>


constexpr int portable_kOpenFlags = O_CLOEXEC;

inline bool portable_isDirectory(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

inline bool portable_isRegularFile(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

/** @brief mkdir(); on Windows the mode is not used (the folder inherits the ACL of its parent). */
inline int portable_makeDirectory(const char* path, unsigned mode) {
    return ::mkdir(path, static_cast<mode_t>(mode));
}

/** @brief fsync() of an open file (_commit on Windows). @return 0 on success. */
inline int portable_syncFile(int fd) {
    return ::fsync(fd);
}

/** @brief fsync() of a folder, so that a rename in it is durable; nothing to do on Windows (NTFS journals it). */
inline bool portable_syncDirectory(const std::string& path) {
    const int fd = ::open(path.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) return false;
    const bool ok = ::fsync(fd) == 0;
    ::close(fd);
    return ok;
}

/** @brief Atomic replacement of @p to by @p from (rename(); on Windows MoveFileEx, which replaces an existing file). */
inline bool portable_replaceFile(const std::string& from, const std::string& to) {
    return ::rename(from.c_str(), to.c_str()) == 0;
}

/** @brief Free (for this user) and total bytes of the file system of @p path. */
inline bool portable_diskSpace(const std::string& path, uint64_t& freeBytes, uint64_t& totalBytes) {
    struct statvfs vfs;
    if (::statvfs(path.c_str(), &vfs) != 0) return false;
    freeBytes = static_cast<uint64_t>(vfs.f_bavail) * vfs.f_frsize;
    totalBytes = static_cast<uint64_t>(vfs.f_blocks) * vfs.f_frsize;
    return true;
}


#endif // _ANDROID_PORTABLE_H__
