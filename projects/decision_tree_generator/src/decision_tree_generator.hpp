#pragma once
#include<map>
#include<string>
#include<vector>
#include<cinttypes>
#include <rules/standart_game_value_node.hpp>

class TDecisionTreeGenerator
{
public:
    using TValue = std::vector<uint8_t>;
    using TValuesList = std::vector<TValue>;
    using TBCPair = std::pair<uint32_t, uint32_t>;
    using TBCDistribution = std::map<TBCPair, TValuesList>;

    TDecisionTreeGenerator() = default;
    ~TDecisionTreeGenerator() = default;

    void setLogLevel(int _log_level)
    {
        m_log_level = _log_level;
    }

    void start();

private:
    TValuesList generateAllPossibleValues();
    TBCDistribution distributeValuesByBullsNCows( TValue const & predictedValue, TValuesList const & values );
    void generatePermutations(std::vector<uint32_t>& current, std::vector<bool>& used, std::vector<std::vector<uint32_t>>& result, uint32_t n);
    std::vector<std::vector<uint32_t>> generateAllPositions(uint32_t n);
    TValuesList generateEquivalentValues(TValuesList const & solvedValues);
    std::vector<TValuesList> generateAllFirstNEquivalentValues(uint32_t N);
    std::set<TDecisionTreeGenerator::TValue> splitingAlgorithm(TValuesList const & availableValues, TValuesList const & checkingValues);
    std::shared_ptr<TValueNode> chooseBestEquivalentValues( TValuesList const & availableValues, TValuesList const & solvedValues );
    std::shared_ptr<TValueNode> customMinimax(TValuesList const & values, int depth);


    std::vector<TValuesList> const & Values3Steps( ) const
    {
        return m_values3Steps;
    }

    int LogLevel() const
    {
        return m_log_level;
    }

private: //attributes
    int m_log_level = 1;
    std::vector<TValuesList> m_values3Steps{};
};
