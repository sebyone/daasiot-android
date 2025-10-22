#ifndef DAAS_WRAPPER_HPP
#define DAAS_WRAPPER_HPP

#include "../include/daas.hpp"  // From src/main/include
#include <string>
#include <vector>
#include <cstdint>

class DaasWrapper {
public:
    DaasWrapper();
    ~DaasWrapper();

    // --- Lifecycle ---
    // Returns pointer handle as jlong on JNI side (we'll return pointer to wrapper instance itself there)
    // Core operations (return daas_error_t as int)
    int doInit(int sid, int din);
    int doEnd();
    int doReset();
    int doPerform(int mode);

    // --- Info / strings ---
    std::string getVersion();
    std::string getBuildInfo();
    std::string listAvailableDrivers();

    // --- Drivers ---
    int enableDriver(unsigned driverId, const std::string& localUri);

    // --- Status / Config ---
    // returns a heap-allocated copy; caller must call freeNodeState(ptr)
    nodestate_t* getStatusCopy();
    uint64_t getSyncedTimestamp();
    bool storeConfiguration(IDepot* depot);
    bool loadConfiguration(IDepot* depot);
    bool doStatisticsReset();
    uint64_t getSystemStatistics(int syscode);

    // --- Mapping & Availability ---
    int map(din_t din);
    int map_with_addr(din_t din, link_t link, const char* suri);
    int map_with_addr_skey(din_t din, link_t link, const char* suri, const char* skey);
    int removeNode(din_t din);
    dinlist_t* listNodesCopy(); // returns heap-allocated copy
    int locate(din_t din);

    // --- Send / Receive / Exchange ---
    int send_status(din_t din);
    // returns pointer to nodestate_t copy (caller frees)
    nodestate_t* statusCopy(din_t din);
    // fetch: returns pointer copy and int status
    nodestate_t* fetchCopy(din_t din, uint16_t opts, int &outErr);

    int setATSMaxError(int32_t error);

    // Real-time session
    bool use(din_t din);
    bool end(din_t din);
    unsigned send(din_t din, const unsigned char* outbound, unsigned size);
    unsigned received(din_t din);
    unsigned receive(din_t din, unsigned char* inbound, unsigned max_size);

    // --- Transfer / typesets / DDOs ---
    tsetlist_t* listTypesetsCopy(); // heap copy
    int pull(din_t din, DDO** outDDO); // outDDO allocated on success (caller frees with freeDDO)
    int push(din_t din, DDO* outboundDDO);
    int availablesPull(din_t din, uint32_t &count);
    int addTypeset(uint16_t typeset_code, uint16_t typeset_size);

    // --- Test / frisbee ---
    int frisbee(din_t din);
    int frisbee_icmp(din_t din, uint32_t timeout, uint32_t retry);
    int frisbee_dperf(din_t din, uint32_t sender_pkt_total, uint32_t block_size, uint32_t sender_trip_period);
    dperf_info_result get_frisbee_dperf_result();

    // --- utility to free returned pointers ---
    static void freeNodeState(nodestate_t* ptr);
    static void freeDDO(DDO* ptr);
    static void freeDinList(dinlist_t* ptr);
    static void freeTsetList(tsetlist_t* ptr);

private:
    DaasAPI daas_;
};

#endif // DAAS_WRAPPER_HPP