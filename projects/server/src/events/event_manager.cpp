#include<events/event_manager.hpp>


TEventManager::TEventManager() : m_next_id(0) {}

int TEventManager::addEvent(const std::string& data)
{
    int id;
    {
        std::lock_guard<std::mutex> lock(m_events_mutex);
        id = m_next_id++;
        auto event = std::make_shared<TEvent>(id, data);
        m_events[id] = event;
        {
            std::lock_guard<std::mutex> q_lock(m_queue_mutex);
            m_event_queue.push(event);
            m_queue_cv.notify_one();
        }
    }

    return id;
}

std::shared_ptr<TEvent> TEventManager::getNextEvent()
{
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    m_queue_cv.wait(lock, [this](){ return !m_event_queue.empty() || m_stopped; });

    if (m_stopped) return nullptr;

    auto event = m_event_queue.front();
    m_event_queue.pop();
    return event;
}

void TEventManager::setEventResponse(int id, const std::string& response)
{
    std::shared_ptr<TEvent> event;
    std::lock_guard<std::mutex> lock(m_events_mutex);
    auto it = m_events.find(id);
    if (it == m_events.end())
    {
        return;
    }
    event = it->second;
    event->setResponse(response);
}

std::pair<bool, std::string> TEventManager::getEventResponse(int id)
{
    std::shared_ptr<TEvent> event;
    {
        std::lock_guard<std::mutex> lock(m_events_mutex);
        auto it = m_events.find(id);
        if (it == m_events.end())
        {
            return std::make_pair( false, "Event not found" );
        }
        event = it->second;
    }
    return  std::make_pair( true, event->waitForResponse() );
}

void TEventManager::removeEvent(int id)
{
    std::lock_guard<std::mutex> lock(m_events_mutex);
    m_events.erase(id);
}

void TEventManager::stop()
{
    m_stopped = true;
    m_queue_cv.notify_all();
}
