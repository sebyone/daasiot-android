/*
 * DaaS-IoT 2019, 2025 (@) Sebyone Srl
 *
 * File: daas.h
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
 * plogiacco@smartlab.it - initial design, implementation and documentation
 * sebastiano.meduri@gmail.com  - initial design, implementation and documentation
 * l.grillo@sebyone.com  - implementation and documentation
 *
 */

#ifndef DAAS_H
#define DAAS_H

#include "daas_types.hpp"

/* ----------------------------------------------------------------------------------------------------------- */

class DaasAPI
{
public:
    DaasAPI();                                    
    DaasAPI(IDaasApiEvent *);                     
    DaasAPI(IDaasApiEvent *, const char *lhver_); 
    ~DaasAPI();

    //////////////////////////////////////////////////////////////////////
    ////  G E N E R A L S                                             ////
    //////////////////////////////////////////////////////////////////////

    const char *listAvailableDrivers();           // Returns drivers list: 2.INET4;3.UART;4.MQTT
    const char *getVersion();                     // returns daas-version
    const char *getBuildInfo();                     // returns local daas-stack details
    
    /**
    @details Releases resources and deactivates the node. 
    @param none
    @returns ERROR_NONE on success, or an error code on failure.
    @end
    */
    daas_error_t doEnd();                         // releases resources and deactivates node
   
    /**
        @details Resets the local node and clears all resources.
        @param none
        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t doReset();                       // reset resources and restarts services
    
    /**
        @details Initializes services and resources for the local node.
        @param sid_: SID of the local node
        @param din_: DIN of the local node
        @returns ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/init/main.cpp init
        @end
    */
    daas_error_t doInit(din_t sid_, din_t din_);  // initializes services and resources (Real-Time or Multi-Threading, release dependent)
    
    /**

        @details Performs the node's task in either real-time or multi-threading mode.
        
        @param mode:
        - PERFORM_CORE_THREAD for multi-threading mode
        - PERFORM_CORE_NO_THREAD for real-time mode
        
        @returns ERROR_NONE on success, or an error code on failure.

        @see doInit
        @end
    */
    daas_error_t doPerform(performs_mode_t mode); // perform node's task ( in RT mode needs to be called cyclically)
    
    /**
        @details Configure driver for network technology (links)
        
        @param driver_id: the communication technology to enable (e.g., _LINK_INET4, _LINK_UART, _LINK_MQTT5)
        @param local_uri: the physical address of the local node (e.g., "192.168.1.1:2020")

        @returns ERROR_NONE on success, or an error code on failure.

        @see doInit
        @end
    */
    daas_error_t enableDriver(link_t driver_id, const char *local_uri); // Configure driver for network technology (links)

    /**
        @details Returns the status of the local node.
       
        @param none
        
        @returns the status of the local node.
        - This includes hardware version, linked channels, synchronization status, security policy, and more.
    
        @end
    */
    nodestate_t getStatus(); // returns local node's instance status
     
    /**
        @details Saves the current configuration to the specified storage interface.
        
        @param storage_interface: the interface to save the configuration (e.g., a file system, database, etc.)
        
        @returns true if the backup was successful, false otherwise.
        
        @see loadConfiguration
        @end
    */
    bool storeConfiguration(IDepot* storage_interface);

    /**
        @details Loads the configuration from the specified storage interface.
        
        @param storage_interface: the interface to load the configuration from (e.g., a file system, database, etc.)
        
        @returns true if the configuration was loaded successfully, false otherwise.
        
        \par Example:
        \snippet examples/restore/main.cpp backup_restore
        @end
    */
    bool loadConfiguration(IDepot* storage_interface);
    
    /**
        @details Resets the system's statistics data.

        @param none

        @returns true if the reset was successful, false otherwise.
        @end
    */
    bool doStatisticsReset();

    /**
        @details Returns the statistics of the local node for a specific system code.

        @param label: the system code to get statistics for (e.g., _cor_dme_sended)

        @returns the system statistics for the given label.
        @end
    */
    uint64_t getSystemStatistics(syscode_t label); 
   
    /* Mapping      -------------------------------------------------------------------------------------------- */

    /**
        @details Maps a new node to the local instance.
       
        @param din: DIN of the node to map
        @param link_: the communication technology to use (e.g., _LINK_INET4, _LINK_UART, _LINK_MQTT5)
        @param suri: the physical address of the node (e.g., "192.168.1.1:2020")

        @returns ERROR_NONE on success, or an error code on failure.

        @see doInit
        @end
    */
    daas_error_t map(din_t din);                                                   // adds new node to local instance
    daas_error_t map(din_t din, link_t link_, const char *suri);                   // adds node-identifier and related physical address ( link: 1="INET4", 2="UART", 3="MQTT5")
    daas_error_t map(din_t din, link_t link_, const char *suri, const char *skey); // adds node-identifier and related physical address ( link: 1="INET4", 2="UART", 3="MQTT5")
    
    /**
        @details Removes a node from the local instance.

        @param din: DIN of the node to remove

        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t remove(din_t din);

    /* Availability -------------------------------------------------------------------------------------------- */
    
    /**
        @details Returns map entries (known nodes) in the local instance.

        @param none

        @returns a list of known nodes (din_t) in the local instance.
        @end
    */
    dinlist_t listNodes();                       // Returns map entries  (knows nodes) ( din1, din2, )
    
    /**
        @details It starts a process to locate the node if it is not inside the known table.
        
        @param din_: DIN of the node to locate

        @returns ERROR_NONE if the node is known, or an error code if it is not.

        @see pull
        @end
    */
    daas_error_t locate(din_t din);
    
    /**
        @details Send the local node's status to a remote node.
        
        @param din: DIN of the remote node to send the status to

        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t send_status(din_t din); // Send local status to remote node (din)
    
    /**
        @details Fetches the status of a remote node.

        @param din: DIN of the remote node to fetch the status from
        
        @returns the nodestate_t of the remote node.
        @end
    */
    const nodestate_t& status(din_t din);

    /**
        @details Fetches the status of a remote node and updates its status in the local instance.
        
        @param din: DIN of the remote node to fetch the status from
        @param opts: options for fetching 
       
        @returns the nodestate_t of the remote node after fetching.
        @end
    */
    const nodestate_t& fetch(din_t din, uint16_t opts);

    /**
        @details Time with ATS correction used to be able to communicate inside the DaaS network.
        
        @param none
        
        @returns the synced timestamp of the local node.
        @end
    */
    uint64_t getSyncedTimestamp();

    /* Security     -------------------------------------------------------------------------------------------- */
    
    /**
        @details Unlocks a remote node by setting its security key.
        
        @param din: DIN of the remote node to unlock
        @param skey: the security key to set
        
        @returns the nodestate_t of the remote node after unlocking.
        @end
    */
    const nodestate_t& unlock(din_t din, const char *skey); 
    
    /**
        @details Set SKEY and security policy for local node
        
        @param skey: the security key to set
        @param policy_: the security policy to set

        @returns the nodestate_t of the local node after setting the security key and policy.
        @end    
    */
    const nodestate_t& lock(const char *skey, unsigned policy_);

    /* Synchronize  -------------------------------------------------------------------------------------------- */
    
    /**
        @details Set the local system time on remote node din and synchronize ATS.

        @param din: DIN of the remote node to synchronize
        @param timezone: the timezone offset in seconds

        @returns the nodestate_t of the remote node after synchronization.
        @end
    */
    const nodestate_t& syncNode(din_t din, unsigned timezone);  

    /**
        @details Set the local system time on remote node din and synchronize ATS

        @param din: DIN of the remote node to synchronize
        @param bubble_time: max error allowed for synchronization in milliseconds

        @returns the nodestate_t of the remote node after synchronization.
        @end
    */
    const nodestate_t& syncNet(din_t din, unsigned bubble_time);

    /**
        @details Set the maximum error allowed for ATS synchronization
        
        @param error: the maximum error in milliseconds
        @end
    */
    void setATSMaxError(int32_t error); // ATS

    /* Exchange     -------------------------------------------------------------------------------------------- */
    
    /**
        @details Starts a real-time session with a remote node.

        @param din: DIN of the remote node to start the session with

        @returns true if the RT session was successfully started, false otherwise. (OPEN CONNECTION!!!!)
        @end
    */
    bool use(din_t din);                                                    

    /**
        @details Ends a real-time session with a remote node.
        
        @param din: DIN of the remote node to end the session with

        @returns true if the RT session was successfully ended, false otherwise.
        @end
    */
    bool end(din_t din);

    /**
        @details Sends data to a remote node in a real-time session.
        
        @param din: DIN of the remote node to send data to
        @param outbound: pointer to the data to send
        @param size: size of the data to send

        @returns the size of data sent.

        @end
    */
    unsigned send(din_t din, unsigned char *outbound, unsigned size);

    /**
        @details Checks if there is data available from a remote node in a real-time session.
        
        @param din: DIN of the remote node to check for data

        @returns the size of data received.

        @end
    */
    unsigned received(din_t din);


    /**
        @details Receives data from a remote node in a real-time session.
        
        @param din: DIN of the remote node to receive data from
        @param inbound: reference to a variable that will hold the received data
        @param max_size: maximum size of data to receive

        @returns the size of data received.

        @end
    */
    unsigned receive(din_t din, unsigned char &inbound, unsigned max_size);

    /* Transfer     -------------------------------------------------------------------------------------------- */
    /**
        @details Returns a list of user-defined typesets.
        
        @param none

        @returns a reference to the list of user-defined typesets.
        
        @note The list is of type tsetlist_t, which is a Vector of typeset
        @end
    */
    tsetlist_t &listTypesets();
   
    /**
        @details Pulls a DDO from a remote node.

        @param din: DIN of the remote node to pull data from
        @param inboundDDO: pointer to a DDO pointer that will hold the pulled DDO
        
        @returns ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/pull/main.cpp pull

        @end
    */
    daas_error_t pull(din_t din, DDO **inboundDDO);

    /**
        @details Pushes a DDO to a remote node.

        @param din: DIN of the remote node to send data to
        @param outboundDDO: pointer to the DDO to send

        @returns ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/push/main.cpp push

        @end
    */
    daas_error_t push(din_t din, DDO *outboundDDO);

    /**
        @details Checks if there are available DDOs from a remote node.

        @param din: DIN of the remote node to check for available DDOs
        @param count: reference to a variable that will hold the number of available DDOs
       
        @returns ERROR_NONE on success, or an error code on failure.

        @see pull

        @end
    */
    daas_error_t availablesPull(din_t din, uint32_t &count);

    /**
        @details Adds a user-defined typeset.

        @param typeset_code: the code of the typeset to add
        @param typeset_size: the size of the typeset in bytes

        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t addTypeset(const uint16_t typeset_code, const uint16_t typeset_size);
    
    
    /* TEST */

    /**
        @details Pings a remote node to check its availability.

        @param din: DIN of the remote node to ping

        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t frisbee(din_t din);               

    /**
        @details Pings a remote node with a specified timeout and retry count.

        @param din: DIN of the remote node to ping
        @param timeout: maximum time to wait for a reply (in milliseconds)
        @param retry: number of retries if no reply is received

        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t frisbee_icmp(din_t din, uint32_t timeout, uint32_t retry); 

    /**
        @details Measures the performance of data transfer to a remote node.
        
        @param din: DIN of the remote node to ping
        @param sender_pkt_total: total number of packets to send
        @param block_size: size of each packet in bytes
        @param sender_trip_period: time period between each packet sent (in milliseconds)
        
        @returns ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t frisbee_dperf(din_t din, uint32_t sender_pkt_total = 10, uint32_t block_size = 1024*1024, uint32_t sender_trip_period = 0); 
    
    /**
        @details Returns the result of a frisbee performance test.
        
        @param none
        
        @returns a dperf_info_result structure containing the performance test results:
        - sender_first_timestamp: timestamp of the first packet sent by the sender
        - local_end_timestamp: timestamp of the last packet received by the sender
        - remote_first_timestamp: timestamp of the first packet received by the receiver
        - remote_last_timestamp: timestamp of the last packet received by the receiver
        - remote_pkt_counter: number of packets received by the receiver
        - remote_data_counter: total data received by the receiver in bytes
        @end
    */
    dperf_info_result get_frisbee_dperf_result();
};

#endif // DAASIOT_H
