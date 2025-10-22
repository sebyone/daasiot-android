#include "daas_wrapper.hpp"
#include <cstring> // for memcpy if needed
#include <iostream>

DaasWrapper::DaasWrapper() {
    // ctor default
}

DaasWrapper::~DaasWrapper() {
    // ensure proper shutdown
    daas_.doEnd();
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
    dinlist_t* copy = new dinlist_t(list); // requires copy ctor in Vector; if not available, you'd implement manual copy
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
    outErr = static_cast<int>(ERROR_NONE); // fetch signature doesn't return error in your header, so we assume success; adjust if real API differs
    nodestate_t* copy = new nodestate_t(ns);
    return copy;
}

int DaasWrapper::setATSMaxError(int32_t error) {
    daas_.setATSMaxError(error);
    return 0;
}

/* Real-time session */
bool DaasWrapper::use(din_t din) {
    return daas_.use(din);
}
bool DaasWrapper::end(din_t din) {
    return daas_.end(din);
}
unsigned DaasWrapper::send(din_t din, const unsigned char* outbound, unsigned size) {
    // daas_.send expects unsigned char*; cast away const
    unsigned char* buf = nullptr;
    if (size > 0) {
        buf = new unsigned char[size];
        memcpy(buf, outbound, size);
    }
    unsigned result = daas_.send(din, buf, size);
    delete[] buf;
    return result;
}
unsigned DaasWrapper::received(din_t din) {
    return daas_.received(din);
}
unsigned DaasWrapper::receive(din_t din, unsigned char* inbound, unsigned max_size) {
    return daas_.receive(din, *inbound, max_size); // note: your signature had `unsigned char &inbound` - adjust as needed
}

/* Transfer / DDOs */
tsetlist_t* DaasWrapper::listTypesetsCopy() {
    tsetlist_t &list = daas_.listTypesets();
    tsetlist_t* copy = new tsetlist_t(list);
    return copy;
}

int DaasWrapper::pull(din_t din, DDO** outDDO) {
    // Call and return error; if success we expect outDDO to be set by API
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

/* Frisbee / test */
int DaasWrapper::frisbee(din_t din) {
    return static_cast<int>(daas_.frisbee(din));
}
int DaasWrapper::frisbee_icmp(din_t din, uint32_t timeout, uint32_t retry) {
    return static_cast<int>(daas_.frisbee_icmp(din, timeout, retry));
}
int DaasWrapper::frisbee_dperf(din_t din, uint32_t sender_pkt_total, uint32_t block_size, uint32_t sender_trip_period) {
    return static_cast<int>(daas_.frisbee_dperf(din, sender_pkt_total, block_size, sender_trip_period));
}
dperf_info_result DaasWrapper::get_frisbee_dperf_result() {
    return daas_.get_frisbee_dperf_result();
}

/* Free helpers */
void DaasWrapper::freeNodeState(nodestate_t* ptr) {
    delete ptr;
}
void DaasWrapper::freeDDO(DDO* ptr) {
    delete ptr;
}
void DaasWrapper::freeDinList(dinlist_t* ptr) {
    delete ptr;
}
void DaasWrapper::freeTsetList(tsetlist_t* ptr) {
    delete ptr;
}