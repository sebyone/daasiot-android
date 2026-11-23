/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_events.hpp
 *
 * daas::sdk::EventRouter: the IDaasApiEvent of a node. Every callback of libdaas becomes an Event delivered to
 * the listeners of its type, queued and run on the application thread (dispatch, Node::service) or run in the
 * callback; a pipe wakes a poll/epoll/select loop.
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
 * pierpaolosestito-dev - daas::sdk of daasiot-esp32, the model of Node, NodeConfig, EventRouter
 *
 * Changelog:
 * 08/11/2026 - plogiacco@sebyone.it - creation (daas_sdk.h): EventRouter queued on the application thread
 * 08/11/2026 - plogiacco@sebyone.it - eventFd for poll/epoll
 * 09/11/2026 - plogiacco@sebyone.it - eventFd on Windows: a loopback UDP socket (SOCKET) for select/WSAPoll
 * 08/11/2026 - plogiacco@sebyone.it - split from the node into gnu_events.hpp
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 */

#ifndef _ANDROID_EVENTS_H__
#define _ANDROID_EVENTS_H__

#include <daas.hpp>
#include <daas_types.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace daas { namespace sdk {

enum class EventType { DinAccepted, DdoReceived, FrisbeeReceived, NodeStateReceived, AtsSyncCompleted,
    FrisbeeDperfCompleted, NetworkDiscovered, NodeConnectedToNetwork, StreamInfoReceived, AttackPanic };

/** @brief Name of the event as in daasiot-java ("ddoReceived", ...). */
const char* eventName(EventType type);

struct Event {
    explicit Event(EventType eventType) : type(eventType) {}
    EventType type;
    din_t din = 0;          ///< din of the event; origin for AttackPanic
    din_t sid = 0;          ///< sid; the neighbour it came from for AttackPanic
    int payloadSize = 0;
    typeset_t typeset = 0;
    link_t link = _LINK_NONE;
    uint32_t value1 = 0;    ///< packets (dperf), stream type (stream)
    uint32_t value2 = 0;    ///< block size (dperf), stream id (stream)
};

/**
 * @brief IDaasApiEvent that hands every callback to the listeners of its type.
 * @details Queued (the default of Node): the callback, on a library thread, only queues the event; the
 *          listeners run on the thread that calls dispatch() (Node::service()), where they may call the
 *          whole API (guide §4.6). Direct: the listeners run inside the callback, so they must be short and
 *          must not call doEnd/doInit.
 */
class EventRouter : public IDaasApiEvent {
public:
    using Listener = std::function<void(const Event&)>;
    void on(EventType type, Listener listener);
    /** @brief Like on(), but the listener runs before the ones already registered. */
    void onFirst(EventType type, Listener listener);
    ~EventRouter() override;
    void setQueued(bool queued) { queued_ = queued; }
    /**
     * @brief A file descriptor that becomes readable when events are queued (a pipe), for an application that
     *        waits in poll/epoll/select: on POLLIN call dispatch(), which also resets it. -1 if not available.
     */
    int eventFd();
    bool queued() const { return queued_; }
    /** @brief Runs the listeners of the queued events; waits up to @p wait for the first one. @return how many. */
    size_t dispatch(std::chrono::milliseconds wait = std::chrono::milliseconds(0));
    /** @brief Wakes a dispatch() that is waiting (e.g. from a signal-driven shutdown). */
    void wake();

    void dinAccepted(din_t din) override;
    void ddoReceived(int size, typeset_t typeset, din_t din) override;
    void frisbeeReceived(din_t din) override;
    void nodeStateReceived(din_t din) override;
    void atsSyncCompleted(din_t din) override;
    void frisbeeDperfCompleted(din_t din, uint32_t packets, uint32_t blockSize) override;
    void networkDiscovered(din_t din, din_t sid, link_t link) override;
    void nodeConnectedToNetwork(din_t sid, din_t din) override;
    void streamInfoReceived(din_t din, stream_type packetType, uint32_t streamId) override;
    void attackPanic(din_t origin, din_t from) override;

private:
    void post(const Event& event);
    void emit(const Event& event) noexcept;
    std::vector<std::pair<EventType, Listener>> listeners_;
    std::atomic<bool> queued_{true};
    std::mutex mutex_;
    std::condition_variable ready_;
    std::deque<Event> queue_;
    bool woken_ = false;
    int fd_ = -1;
    int wfd_ = -1;              ///< the writable end of the pipe of fd_
};

}} // namespace daas::sdk

#endif // _ANDROID_EVENTS_H__
