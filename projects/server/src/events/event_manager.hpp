#pragma once
#include <atomic>
#include <unordered_map>
#include <queue>
#include <memory>

#include <events/event.hpp>

class TEventManager
{
public:
    TEventManager();

    int addEvent(const std::string& data);
    std::shared_ptr<TEvent> getNextEvent();
    void setEventResponse(int id, const std::string& response);
    std::string getEventResponse(int id);
    void removeEvent(int id);
    void stop();

private:
    std::atomic<int> m_next_id;
    std::unordered_map<int, std::shared_ptr<TEvent>> m_events;
    std::queue<std::shared_ptr<TEvent>> m_event_queue;
    std::mutex m_events_mutex;
    std::mutex m_queue_mutex;
    std::condition_variable m_queue_cv;
    std::atomic<bool> m_stopped{false};
};
