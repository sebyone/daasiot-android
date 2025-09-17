#include "daas_wrapper.hpp"
#include <iostream>

// Default constructor
DaasWrapper::DaasWrapper() = default;

// Destructor with cleanup
DaasWrapper::~DaasWrapper() {
    if (initialized) {
        std::cout << "[DaasWrapper] Cleaning up resources..." << std::endl;
        daasInstance.doEnd();
    }
}

// Initialize the core DaaS system
bool DaasWrapper::initCore(int sid, int din) {
    daas_error_t result = daasInstance.doInit(sid, din);
    if (result != ERROR_NONE) {
        std::cerr << "[DaasWrapper] Failed to initialize core. Error: " << result << std::endl;
        return false;
    }
    initialized = true;
    std::cout << "[DaasWrapper] Core initialized successfully (SID=" << sid << ", DIN=" << din << ")" << std::endl;
    return true;
}

// Enable a communication driver
bool DaasWrapper::enableDriver(link_t driverId, const std::string& localUri) {
    daas_error_t result = daasInstance.enableDriver(driverId, localUri.c_str());
    if (result != ERROR_NONE) {
        std::cerr << "[DaasWrapper] Failed to enable driver. Error: " << result << std::endl;
        return false;
    }
    std::cout << "[DaasWrapper] Driver enabled successfully at URI: " << localUri << std::endl;
    return true;
}

// Map a new remote node
bool DaasWrapper::mapNode(int remoteDin, link_t link, const std::string& uri) {
    daas_error_t result = daasInstance.map(remoteDin, link, uri.c_str());
    if (result != ERROR_NONE) {
        std::cerr << "[DaasWrapper] Failed to map node (DIN=" << remoteDin << "). Error: " << result << std::endl;
        return false;
    }
    std::cout << "[DaasWrapper] Node mapped successfully: DIN=" << remoteDin << " at " << uri << std::endl;
    return true;
}

// Return reference to internal API object
DaasAPI& DaasWrapper::getAPI() {
    return daasInstance;
}

// Retrieve version string
std::string DaasWrapper::getVersion() {
    const char* version = daasInstance.getVersion();
    return version ? std::string(version) : "Unknown Version";
}

// Retrieve available drivers
std::string DaasWrapper::getAvailableDrivers() {
    const char* drivers = daasInstance.listAvailableDrivers();
    return drivers ? std::string(drivers) : "No Drivers Found";
}