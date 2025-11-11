#include "daas_wrapper.hpp"
#include <cstring> // for memcpy
#include <iostream>
#include <vector>
#include <memory>

DaasWrapper::DaasWrapper() {
    // ctor default
}

DaasWrapper::~DaasWrapper() {
    // ensure proper shutdown
    try {
        daas_.doEnd();
    } catch (...) {
        // swallow exceptions in destructor
    }
}

/* Lifecycle */
int DaasWrapper::doInit(int sid, int din) {
    return static_cast<int>(daas_.doInit(static_cast<din_t>(sid), static_cast<din_t>(din)));
}
int DaasWrapper::doEnd() {
    return static_cast<int>(daas_.doEnd());
}
int DaasWrapper::doReset() {
    return static_cast<int>(daas_.doReset());
}
int DaasWrapper::doPerform(int mode) {
    return static_cast<int>(daas_.doPerform(static_cast<performs_mode_t>(mode)));
}

/* Info */
std::string DaasWrapper::getVersion() {
    const char* v = daas_.getVersion();
    return v ? std::string(v) : std::string();
}
std::string DaasWrapper::getBuildInfo() {
    const char* b = daas_.getBuildInfo();
    return b ? std::string(b) : std::string();
}
std::string DaasWrapper::listAvailableDrivers() {
    const char* d = daas_.listAvailableDrivers();
    return d ? std::string(d) : std::string();
}

/* Drivers */
int DaasWrapper::enableDriver(unsigned driverId, const std::string& localUri) {
    return static_cast<int>(daas_.enableDriver(static_cast<link_t>(driverId), localUri.c_str()));
}

/* Status & config */
nodestate_t* DaasWrapper::getStatusCopy() {
    nodestate_t st = daas_.getStatus();
    nodestate_t* copy = new nodestate_t();
    *copy = st; // aggregate copy (POD-ish)
    return copy;
}

uint64_t DaasWrapper::getSyncedTimestamp() {
    return daas_.getSyncedTimestamp();
}

bool DaasWrapper::storeConfiguration(IDepot* depot) {
    return daas_.storeConfiguration(depot);
}

bool DaasWrapper::loadConfiguration(IDepot* depot) {
    return daas_.loadConfiguration(depot);
}

bool DaasWrapper::doStatisticsReset() {
    return daas_.doStatisticsReset();
}

uint64_t DaasWrapper::getSystemStatistics(int syscode) {
    return daas_.getSystemStatistics(static_cast<syscode_t>(syscode));
}

/* Mapping & Availability */
int DaasWrapper::map(din_t din) {
    return static_cast<int>(daas_.map(din));
}
int DaasWrapper::map_with_addr(din_t din, link_t link, const char* suri) {
    return static_cast<int>(daas_.map(din, link, suri));
}
int DaasWrapper::map_with_addr_skey(din_t din, link_t link, const char* suri, const char* skey) {
    return static_cast<int>(daas_.map(din, link, suri, skey));
}
int DaasWrapper::removeNode(din_t din) {
    return static_cast<int>(daas_.remove(din));
}
dinlist_t* DaasWrapper::listNodesCopy() {
    dinlist_t list = daas_.listNodes();
    dinlist_t* copy = new dinlist_t(list); // assumes Vector has copy constructor
    return copy;
}
int DaasWrapper::locate(din_t din) {
    return static_cast<int>(daas_.locate(din));
}

/* Exchange */
int DaasWrapper::send_status(din_t din) {
    return static_cast<int>(daas_.send_status(din));
}
nodestate_t* DaasWrapper::statusCopy(din_t din) {
    const nodestate_t& ns = daas_.status(din);
    nodestate_t* copy = new nodestate_t(ns);
    return copy;
}
nodestate_t* DaasWrapper::fetchCopy(din_t din, uint16_t opts, int &outErr) {
    const nodestate_t& ns = daas_.fetch(din, opts);
    outErr = static_cast<int>(ERROR_NONE); // adjust if real API returns errors differently
    nodestate_t* copy = new nodestate_t(ns);
    return copy;
}

/* Real-time session */
bool DaasWrapper::use(din_t din) {
    return daas_.use(din);
}
bool DaasWrapper::end(din_t din) {
    return daas_.end(din);
}
unsigned DaasWrapper::send(din_t din, const unsigned char* outbound, unsigned size) {
    // daas_.send expects unsigned char*; copy into temporary buffer (safer)
    unsigned result = 0;
    if (size == 0 || outbound == nullptr) return 0;
    std::unique_ptr<unsigned char[]> buf(new unsigned char[size]);
    memcpy(buf.get(), outbound, size);
    result = daas_.send(din, buf.get(), size);
    return result;
}
unsigned DaasWrapper::received(din_t din) {
    return daas_.received(din);
}
unsigned DaasWrapper::receive(din_t din, unsigned char* inbound, unsigned max_size) {
    if (!inbound || max_size == 0) return 0;
    // The SDK signature in header is strange (unsigned receive(din_t, unsigned char&, unsigned))
    // We'll assume there's an API overload that fills a buffer: if not, adapt this.
    // Here we call receive(din, inbound_buffer, max_size) if available; else call byte-by-byte.
    // Try the buffer version first (if exists)
    #ifdef HAS_BUFFER_RECEIVE
        return daas_.receive(din, inbound, max_size); // if SDK supports it
    #else
        // Fallback: call receive per-byte (not ideal). We'll simulate by calling SDK's receive with reference to first byte.
        unsigned received = daas_.receive(din, *inbound, max_size);
        return received;
    #endif
}

/* Transfer / DDOs */

// Create a new DDO on heap and return pointer
DDO* DaasWrapper::createDDO(typeset_t typeset, stime_t timestamp) {
    DDO* d = new DDO(typeset, timestamp);
    return d;
}

void DaasWrapper::freeDDO(DDO* ptr) {
    delete ptr;
}

bool DaasWrapper::ddo_setOrigin(DDO* ddo, din_t origin) {
    if (!ddo) return false;
    ddo->setOrigin(origin);
    return true;
}
bool DaasWrapper::ddo_setTypeset(DDO* ddo, typeset_t typeset) {
    if (!ddo) return false;
    ddo->setTypeset(typeset);
    return true;
}
bool DaasWrapper::ddo_setTimestamp(DDO* ddo, stime_t ts) {
    if (!ddo) return false;
    ddo->setTimestamp(ts);
    return true;
}

uint32_t DaasWrapper::ddo_setPayload(DDO* ddo, const uint8_t* data, uint32_t size) {
    if (!ddo) return 0;
    return ddo->setPayload(data, size);
}

uint32_t DaasWrapper::ddo_getPayloadSize(DDO* ddo) {
    if (!ddo) return 0;
    return ddo->getPayloadSize();
}

uint32_t DaasWrapper::ddo_getPayloadAsBinary(DDO* ddo, uint8_t* buffer, uint32_t maxSize) {
    if (!ddo || !buffer) return 0;
    return ddo->getPayloadAsBinary(buffer, 0, maxSize);
}

// Convert daas_.listNodes() to std::vector<din_t>
std::vector<din_t> DaasWrapper::listNodesAsVector() {
    dinlist_t list = daas_.listNodes();
    std::vector<din_t> out;
    // if Vector implements size() and operator[]
    for (uint32_t i = 0; i < list.size(); ++i) {
        out.push_back(list[i]);
    }
    return out;
}

// Convert daas_.listTypesets() to std::vector<typeset_t>
std::vector<typeset_t> DaasWrapper::listTypesetsAsVector() {
    tsetlist_t list = daas_.listTypesets();
    std::vector<typeset_t> out;
    for (uint32_t i = 0; i < list.size(); ++i) {
        out.push_back(list[i]);
    }
    return out;
}

// Pull / Push / availables
int DaasWrapper::pull(din_t din, DDO** outDDO) {
    daas_error_t err = daas_.pull(din, outDDO);
    return static_cast<int>(err);
}

int DaasWrapper::push(din_t din, DDO* outboundDDO) {
    return static_cast<int>(daas_.push(din, outboundDDO));
}

int DaasWrapper::availablesPull(din_t din, uint32_t &count) {
    return static_cast<int>(daas_.availablesPull(din, count));
}

int DaasWrapper::addTypeset(uint16_t typeset_code, uint16_t typeset_size) {
    return static_cast<int>(daas_.addTypeset(typeset_code, typeset_size));
}

/* Sync / Lock / Unlock */

// unlock: return heap-allocated copy and set outErr
nodestate_t* DaasWrapper::unlock(din_t din, const char* skey, int& outErr) {
    const nodestate_t& ns = daas_.unlock(din, skey);
    outErr = static_cast<int>(ERROR_NONE);
    return new nodestate_t(ns);
}

// lock: set skey and policy for local node, return heap copy
nodestate_t* DaasWrapper::lock(const char* skey, unsigned policy_, int& outErr) {
    const nodestate_t& ns = daas_.lock(skey, policy_);
    outErr = static_cast<int>(ERROR_NONE);
    return new nodestate_t(ns);
}

nodestate_t* DaasWrapper::syncNode(din_t din, unsigned timezone, int& outErr) {
    const nodestate_t& ns = daas_.syncNode(din, timezone);
    outErr = static_cast<int>(ERROR_NONE);
    return new nodestate_t(ns);
}

nodestate_t* DaasWrapper::syncNet(din_t din, unsigned bubble_time, int& outErr) {
    const nodestate_t& ns = daas_.syncNet(din, bubble_time);
    outErr = static_cast<int>(ERROR_NONE);
    return new nodestate_t(ns);
}

/* Frisbee / test */
int DaasWrapper::frisbee(din_t din) {
    return static_cast<int>(daas_.frisbee(din));
}
// TODO: Implement frisbee_icmp
// int DaasWrapper::frisbee_icmp(din_t din, uint32_t timeout, uint32_t retry) {
//     return static_cast<int>(daas_.frisbee_icmp(din, timeout, retry));
// }
// int DaasWrapper::frisbee_dperf(din_t din, uint32_t sender_pkt_total, uint32_t block_size, uint32_t sender_trip_period) {
//     return static_cast<int>(daas_.frisbee_dperf(din, sender_pkt_total, block_size, sender_trip_period));
// }
// dperf_info_result DaasWrapper::get_frisbee_dperf_result() {
//     return daas_.get_frisbee_dperf_result();
// }

/* Free helpers */
void DaasWrapper::freeNodeState(nodestate_t* ptr) {
    delete ptr;
}
void DaasWrapper::freeDinList(dinlist_t* ptr) {
    delete ptr;
}
void DaasWrapper::freeTsetList(tsetlist_t* ptr) {
    delete ptr;
}
