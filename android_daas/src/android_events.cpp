/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_events.cpp
 *
 * Implementation of EventRouter.
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
 * 08/11/2026 - plogiacco@sebyone.it - creation (daas_sdk.cpp)
 * 08/11/2026 - plogiacco@sebyone.it - split from the node into gnu_events.cpp
 * 09/11/2026 - plogiacco@sebyone.it - Windows (windows-x86_64-mingw32): eventFd() on a loopback UDP socket pair
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 * 24/11/2026 - l.grillo@sebyone.it - no namespace: the API of android_daas is global, like libdaas
 */

#include "android_events.hpp"

#include <cerrno>

#include <fcntl.h>
#include <unistd.h>


namespace {
// A non-blocking pipe (as on macOS; eventfd would also do on Linux/Android), readable end in fd (for poll/epoll/select), writable end in wfd.
bool openSignal(int& fd, int& wfd) {
    int p[2];
    if (::pipe(p) != 0) return false;
    for (int x : p) {
        ::fcntl(x, F_SETFL, ::fcntl(x, F_GETFL) | O_NONBLOCK);
        ::fcntl(x, F_SETFD, FD_CLOEXEC);
    }
    fd = p[0];
    wfd = p[1];
    return true;
}
void raiseSignal(int, int wfd) { const char one = 1; (void)!::write(wfd, &one, 1); }   // a full pipe already signals
void drain(int fd) { char buffer[64]; while (::read(fd, buffer, sizeof(buffer)) > 0) {} }
void closeSignal(int fd, int wfd) { ::close(fd); ::close(wfd); }
} // namespace

const char* eventName(EventType type) {
    switch (type) {
        case EventType::DinAccepted: return "dinAccepted";
        case EventType::DdoReceived: return "ddoReceived";
        case EventType::FrisbeeReceived: return "frisbeeReceived";
        case EventType::NodeStateReceived: return "nodeStateReceived";
        case EventType::AtsSyncCompleted: return "atsSyncCompleted";
        case EventType::FrisbeeDperfCompleted: return "frisbeeDperfCompleted";
        case EventType::NetworkDiscovered: return "networkDiscovered";
        case EventType::NodeConnectedToNetwork: return "nodeConnectedToNetwork";
        case EventType::StreamInfoReceived: return "streamInfoReceived";
        case EventType::AttackPanic: return "attackPanic";
    }
    return "unknown";
}

void EventRouter::on(EventType type, Listener listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back({type, std::move(listener)});
}

void EventRouter::onFirst(EventType type, Listener listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.insert(listeners_.begin(), {type, std::move(listener)});
}

void EventRouter::emit(const Event& event) noexcept {
    std::vector<Listener> targets;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& entry : listeners_) if (entry.first == event.type) targets.push_back(entry.second);
    }
    for (auto& listener : targets) {
        try { listener(event); } catch (...) {}
    }
}

EventRouter::~EventRouter() {
    if (fd_ >= 0) closeSignal(fd_, wfd_);
}

int EventRouter::eventFd() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ < 0) {
        int fd = -1, wfd = -1;
        if (openSignal(fd, wfd)) { fd_ = fd; wfd_ = wfd; }
        if (fd_ >= 0 && !queue_.empty()) raiseSignal(fd_, wfd_);
    }
    return fd_;
}

void EventRouter::post(const Event& event) {
    if (!queued_) { emit(event); return; }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(event);
        if (fd_ >= 0) raiseSignal(fd_, wfd_);
    }
    ready_.notify_one();
}

size_t EventRouter::dispatch(std::chrono::milliseconds wait) {
    std::deque<Event> batch;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty() && wait.count() > 0)
            ready_.wait_for(lock, wait, [this] { return !queue_.empty() || woken_; });
        woken_ = false;
        batch.swap(queue_);
        if (fd_ >= 0) drain(fd_);                   // readable again on the next post
    }
    for (const auto& event : batch) emit(event);
    return batch.size();
}

void EventRouter::wake() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        woken_ = true;
    }
    ready_.notify_all();
}

void EventRouter::dinAccepted(din_t din) { Event e{EventType::DinAccepted}; e.din = din; post(e); }
void EventRouter::ddoReceived(int size, typeset_t typeset, din_t din) { Event e{EventType::DdoReceived}; e.din = din; e.payloadSize = size; e.typeset = typeset; post(e); }
void EventRouter::frisbeeReceived(din_t din) { Event e{EventType::FrisbeeReceived}; e.din = din; post(e); }
void EventRouter::nodeStateReceived(din_t din) { Event e{EventType::NodeStateReceived}; e.din = din; post(e); }
void EventRouter::atsSyncCompleted(din_t din) { Event e{EventType::AtsSyncCompleted}; e.din = din; post(e); }
void EventRouter::frisbeeDperfCompleted(din_t din, uint32_t packets, uint32_t block) { Event e{EventType::FrisbeeDperfCompleted}; e.din = din; e.value1 = packets; e.value2 = block; post(e); }
void EventRouter::networkDiscovered(din_t din, din_t sid, link_t link) { Event e{EventType::NetworkDiscovered}; e.din = din; e.sid = sid; e.link = link; post(e); }
void EventRouter::nodeConnectedToNetwork(din_t sid, din_t din) { Event e{EventType::NodeConnectedToNetwork}; e.sid = sid; e.din = din; post(e); }
void EventRouter::streamInfoReceived(din_t din, stream_type type, uint32_t id) { Event e{EventType::StreamInfoReceived}; e.din = din; e.value1 = static_cast<uint32_t>(type); e.value2 = id; post(e); }
void EventRouter::attackPanic(din_t origin, din_t from) { Event e{EventType::AttackPanic}; e.din = origin; e.sid = from; post(e); }

