#ifndef DAAS_WRAPPER_HPP
#define DAAS_WRAPPER_HPP

#include "../include/daas.hpp"  // From src/main/include
#include <string>

// High-level wrapper for DaasAPI
class DaasWrapper {
public:
    // Constructor & Destructor
    DaasWrapper();
    ~DaasWrapper();

    // Core initialization
    bool initCore(int sid, int din);

    // Driver handling
    bool enableDriver(link_t driverId, const std::string& localUri);

    // Mapping
    bool mapNode(int remoteDin, link_t link, const std::string& uri);

    // Accessors
    DaasAPI& getAPI();
    std::string getVersion();
    std::string getAvailableDrivers();

private:
    DaasAPI daasInstance;
    bool initialized = false;
};

#endif // DAAS_WRAPPER_HPP