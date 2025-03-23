#pragma once
#include<map>
#include<string>
#include<vector>
#include<cinttypes>
#include <rules/standart_game_value_node.hpp>

class TMinimaxController
{
public:
    using TValue = std::vector<uint8_t>;
    using TValuesList = std::vector<TValue>;
    using TBCPair = std::pair<uint32_t, uint32_t>;
    using TBCDistribution = std::map<TBCPair, TValuesList>;

    TMinimaxController() = delete;
    TMinimaxController(int32_t number);
    ~TMinimaxController() = default;

    void setLogLevel(int _log_level)
    {
        m_log_level = _log_level;
    }

    void start();

private:
    TValuesList generateAllPossibleValues();
    TBCDistribution distributeValuesByBullsNCows( TValue const & predictedValue, TValuesList const & values );
    std::shared_ptr<TValueNode> customMinimax(TValuesList const & values, int depth);
    void saveNumber(TValue const & number, double steps);
    void loadCashNumbers();


    std::map<TValue, double> const & CashNumbers() const
    {
        return m_numbersCash;
    }

    int LogLevel() const
    {
        return m_log_level;
    }

private: //attributes
    std::map<TValue, double> m_numbersCash{};
    int32_t m_cashed_attempt = 2;
    std::string m_filename{};
    int m_log_level = 1;
};
