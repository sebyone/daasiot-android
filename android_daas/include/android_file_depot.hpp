/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_file_depot.hpp
 *
 * file_depot: IDepot on the Android file system (the files folder of the app), for DaasAPI::storeConfiguration and
 * loadConfiguration (libdaas-refs §16.2, guide §14): one file per record key in a state directory, written to
 * a temporary file, fsync'ed and renamed. stateDirectory() gives the directory of a service or of a user.
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
 * 08/11/2026 - plogiacco@sebyone.it - creation (persistence/file_depot.h): same contract as nvs_storage of daasiot-esp32
 * 08/11/2026 - plogiacco@sebyone.it - part of android_daas (was gnu-persistence), renamed gnu_file_depot.hpp
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 * 24/11/2026 - l.grillo@sebyone.it - no namespace: the API of android_daas is global, like libdaas
 */

#ifndef _ANDROID_FILE_DEPOT_H__
#define _ANDROID_FILE_DEPOT_H__

#include "android_defaults.hpp"

#include <daas_types.hpp>

#include <cstdint>
#include <string>


/**
 * @brief State directory of @p app inside @p filesDir, the private files folder of the Android app
 *        (Context.getFilesDir(), e.g. /data/user/0/<package>/files): "<filesDir>/<app>". Not created.
 */
std::string stateDirectory(const std::string& filesDir, const std::string& app);

/** @brief mkdir -p with mode 0700. */
bool makeDirectories(const std::string& path);

/** @brief Writes @p size bytes to @p path atomically: temporary file, fsync, rename, fsync of the folder. */
bool writeFileAtomic(const std::string& path, const void* data, size_t size);

/**
 * @brief IDepot on files: record oKey is "<directory>/k<oKey>.rec".
 * @details open(read) fails when nothing was saved (no record k0), so DaasAPI::loadConfiguration returns false
 *          instead of restoring an empty configuration, as with nvs_storage on ESP32.
 */
class file_depot final : public IDepot {
public:
    explicit file_depot(std::string directory);
    ~file_depot();   // IDepot has no virtual destructor: never delete a file_depot through an IDepot*

    bool open(bool read_mode) override;
    bool close() override;
    bool clearSpace() override;
    bool getSpaceInfo() override;
    bool trash(unsigned oKey) override;
    unsigned save(unsigned oKey, unsigned char* pdata, unsigned dsize) override;
    unsigned load(unsigned oKey, unsigned char* pdata, unsigned dsize) override;

    /** @brief True if a configuration saved by DaasAPI::storeConfiguration is present. */
    bool hasConfiguration();
    const std::string& directory() const { return directory_; }
    /** @brief Bytes free and total of the file system, after getSpaceInfo(). */
    uint64_t freeBytes() const { return freeBytes_; }
    uint64_t totalBytes() const { return totalBytes_; }

private:
    std::string pathOf(unsigned oKey) const;
    std::string directory_;
    bool open_ = false;
    bool writable_ = false;
    uint64_t freeBytes_ = 0;
    uint64_t totalBytes_ = 0;
};


#endif // _ANDROID_FILE_DEPOT_H__
