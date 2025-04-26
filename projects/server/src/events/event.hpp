#pragma once
#include <string>
#include <condition_variable>
#include <mutex>
#include <components/enums.hpp>

class TEvent
{
public:
    TEvent(int id, const std::string& data) : m_id(id), m_data(data), m_completed(false) {}

    int getId() const { return m_id; }
    std::string getData() const { return m_data; }

    void setResponse(const std::string& response)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_response = response;
        m_completed = true;
        m_cv.notify_one();
    }

    std::string waitForResponse()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this](){ return m_completed; });
        return m_response;
    }

private:
    int m_id;
    std::string m_data;
    std::string m_response;
    bool m_completed;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};
