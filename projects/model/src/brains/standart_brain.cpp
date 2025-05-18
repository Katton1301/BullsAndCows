#include <brains/standart_brain.hpp>
#include <player_process.hpp>
#include <rules/standart_game_value_node.hpp>
#include <algorithm>
#include <numeric>
#include <queue>
#include <list>

std::string TDecisionTreeBrain::DecisionTreePath = "./bc.json";

TStandartBrain::TStandartBrain( TStandartPlayerProcess const * _playerProcess )
    : m_playerProcess_cptr(_playerProcess)
    , m_predictedValue(nullptr)
{
}

std::shared_ptr<TGameValue<uint8_t>> const & TStandartBrain::PredictedValue() const
{
    return m_predictedValue;
}

static std::shared_ptr<TValueNode > MainNode()
{
    static std::shared_ptr<TValueNode> mainNode = nullptr;
    if(!mainNode)
    {
        std::vector< uint8_t > firstValue{0,1,2,3};
        mainNode = JSON_TOOLS::loadNodeFromJson( TDecisionTreeBrain::DecisionTreePath, firstValue );
    }
    return mainNode;
}

TDecisionTreeBrain::TDecisionTreeBrain( TStandartPlayerProcess const * _playerProcess )
    : TStandartBrain(_playerProcess)
{
    initDigitCoins();
}

void TDecisionTreeBrain::initDigitCoins()
{
    m_digitCoins.clear();
    for(uint8_t digit = 0; digit < TStandartRules::Instance().NumbersCount(); ++digit)
    {
        m_digitCoins.push_back(std::make_pair(digit,digit));
    }

    for(uint32_t noDigit = 0; noDigit < TStandartRules::Instance().NumbersCount(); ++noDigit)
    {
        uint32_t offsetDigit = m_playerProcess_cptr->GetRandom()(m_digitCoins.size() - noDigit);
        std::swap(m_digitCoins[noDigit].second, m_digitCoins[noDigit + offsetDigit].second);
    }
}

void TDecisionTreeBrain::Init()
{
    m_gameNode = MainNode();
}

void TDecisionTreeBrain::makePredict()
{
    std::vector<uint8_t> predectedValue;
    if(m_playerProcess_cptr->HistoryList().size() > 0 )
    {
        auto bc = m_playerProcess_cptr->HistoryList().back().second;
        m_gameNode = m_gameNode->ChildsAt(bc.first, bc.second);
    }
    predectedValue = m_gameNode->Value();
    flipValueCoins(predectedValue,false);
    m_predictedValue = std::make_shared<TGameValue<uint8_t>>(predectedValue);
}

void TDecisionTreeBrain::restoreFromHistory()
{
    auto const & history = m_playerProcess_cptr->HistoryList();
    Init();

    m_digitCoins.clear();
    for(uint8_t digit = 0; digit < TStandartRules::Instance().NumbersCount(); ++digit)
    {
        m_digitCoins.push_back(std::make_pair(digit,TStandartRules::Instance().NumbersCount()));
    }

    for (auto const & [value, bc] : history)
    {
        for(uint32_t i = 0; i < value.List().size(); ++i)
        {
            auto digit = m_gameNode->Value()[i];
            auto it = std::find_if(m_digitCoins.begin(), m_digitCoins.end(), [digit](auto const & pair){return pair.first == digit;});
            assert(it == m_digitCoins.end());
            it->second = value.List()[i];
        }
        if (m_gameNode->ContainBullsNCows(bc.first, bc.second))
        {
            m_gameNode = m_gameNode->ChildsAt(bc.first, bc.second);
        }
        else
        {
            Init();
            break;
        }
    }

    std::set<uint8_t> usedDigits;
    for(auto const & [front, back] : m_digitCoins)
    {
        if(back != TStandartRules::Instance().NumbersCount())
        {
            usedDigits.emplace(back);
        }
    }
    std::vector<uint8_t> unusedDigits;
    for(uint8_t digit = 0; digit < TStandartRules::Instance().NumbersCount(); ++digit)
    {
        if(!usedDigits.contains(digit))
        {
            unusedDigits.push_back(digit);
        }
    }
    for(auto & [front, back] : m_digitCoins)
    {
        if(back == 10)
        {
            uint32_t offsetDigit = m_playerProcess_cptr->GetRandom()(unusedDigits.size());
            std::swap(unusedDigits[offsetDigit], unusedDigits.back());
            back = unusedDigits.back();
            unusedDigits.pop_back();
        }
    }
    assert(unusedDigits.empty());

}

void TDecisionTreeBrain::flipValueCoins(std::vector<uint8_t> & value, bool isFrontSide )
{
    if(isFrontSide)
    {
        for(auto & digit : value)
        {
            auto it = std::ranges::find_if(
                m_digitCoins.begin(),
                m_digitCoins.end(),
                [&digit]( auto const & coin )
                {
                    return coin.first == digit;
                });
            assert(it != m_digitCoins.end());
            digit = (*it).second;
        }
    }
    else
    {
        for(auto & digit : value)
        {
            auto it = std::ranges::find_if(
                m_digitCoins.begin(),
                m_digitCoins.end(),
                [&digit]( auto const & coin )
                {
                    return coin.second == digit;
                });
            assert(it != m_digitCoins.end());
            digit = (*it).first;
        }
    }
}

TAnaliticBrain::TAnaliticBrain( TStandartPlayerProcess const * _playerProcess )
    : TStandartBrain(_playerProcess)
    , m_possibleValues()
{
}

void TAnaliticBrain::Init( )
{
    copyPossibleValuesList();
}

void TAnaliticBrain::copyPossibleValuesList( )
{
    m_possibleValues.clear();
    m_possibleValues = TStandartRules::Instance().AllPossibleGameValues();
}

TStandartRandomBrain::TStandartRandomBrain( TStandartPlayerProcess const * _playerProcess )
    : TAnaliticBrain(_playerProcess)
{
}

void TStandartRandomBrain::makePredict( )
{
    assert(m_possibleValues.size() > 0);
    auto rnd_offset = m_playerProcess_cptr->GetRandom()(m_possibleValues.size());
    m_predictedValue = std::make_shared<TGameValue<uint8_t>>(m_possibleValues[rnd_offset]);
    m_possibleValues.erase( m_possibleValues.begin() + rnd_offset );
}

void TStandartRandomBrain::restoreFromHistory()
{
    Init();
    auto const & history = m_playerProcess_cptr->HistoryList();
    for(auto const & [value, bc] : history)
    {
        auto const & list = value.List();
        std::erase_if(m_possibleValues,
                    [list](auto const & possibleValue){return possibleValue.List() == list;}
        );
    }
}

TStandartStupidBrain::TStandartStupidBrain( TStandartPlayerProcess const * _playerProcess )
    : TAnaliticBrain(_playerProcess)
{
}

void TStandartStupidBrain::Init()
{
    TAnaliticBrain::Init();
    m_digitsPriority.clear();
    for(uint8_t i = 0; i < TStandartRules::Instance().NumbersCount(); ++i)
    {
        m_digitsPriority.emplace(i,0);
    }
}

void TStandartStupidBrain::makePredict( )
{
    assert(m_possibleValues.size() > 0);
    if(m_playerProcess_cptr->HistoryList().size() > 0 )
    {
        handleValuesByHistory( );
    }

    int32_t chosen_value_offset = -1;
    if(m_playerProcess_cptr->HistoryList().size() < 2) //collect information
    {
        chosen_value_offset = chooseFirstAndSecondGameValueIndex();
    }

    if(chosen_value_offset < 0)
    {
        chosen_value_offset = chooseBestGameValueOffset();
    }
    assert(chosen_value_offset >= 0);
    m_predictedValue = std::make_shared<TGameValue<uint8_t>>(m_possibleValues[chosen_value_offset]);
    m_possibleValues.erase( m_possibleValues.begin() + chosen_value_offset );
}

void TStandartStupidBrain::restoreFromHistory()
{
    auto const & history = m_playerProcess_cptr->HistoryList();
    Init();
    for (const auto& [value, bc] : history)
    {
        if (bc.first + bc.second == 0)
        {
            eraseValuesForDigits(value.List());
        } else if (bc.first + bc.second == TStandartRules::Instance().ValueSize())
        {
            leaveValuesForDigits(value.List());
        }
        calcPriority();
    }
}

void TStandartStupidBrain::handleValuesByHistory( )
{
    auto const & handledValue = m_playerProcess_cptr->HistoryList().back();
    if(handledValue.second.first + handledValue.second.second == 0)
    {
        eraseValuesForDigits(handledValue.first.List());
    }
    if(handledValue.second.first + handledValue.second.second == TStandartRules::Instance().ValueSize())
    {
        leaveValuesForDigits(handledValue.first.List());
    }
}

void TStandartStupidBrain::eraseValuesForDigits( std::vector<uint8_t> const & digits )
{
    auto it = m_possibleValues.begin();
    while( it != m_possibleValues.end() )
    {
        if(
            std::ranges::find_if(
                it->List().begin(),
                it->List().end(),
                [&digits]( auto d )
                {
                    return std::find(digits.begin(),digits.end(),d) != digits.end();
                }
            ) != it->List().end()
        )
        {
            it = m_possibleValues.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void TStandartStupidBrain::leaveValuesForDigits( std::vector<uint8_t> const & digits )
{
    auto it = m_possibleValues.begin();
    while( it != m_possibleValues.end() )
    {
        if(
            std::accumulate(
                it->List().begin(),
                it->List().end(),
                0u,
                [&digits]( auto sum, auto d )
                {
                    return sum + (std::find(digits.begin(),digits.end(),d) != digits.end());
                }
            ) < digits.size()
        )
        {
            it = m_possibleValues.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

int32_t TStandartStupidBrain::chooseFirstAndSecondGameValueIndex()
{
    int32_t chosen_value_offset = -1;
    if(m_playerProcess_cptr->HistoryList().size() == 0) //choose first
    {
        chosen_value_offset = m_playerProcess_cptr->GetRandom()(m_possibleValues.size());
    }
    else
    {
        auto const & digits = m_playerProcess_cptr->HistoryList().back().first.List();
        std::vector< uint32_t > bestValueIndexes;
        for(uint32_t i = 0; i < m_possibleValues.size(); ++i)
        {
            if(
                std::ranges::find_if(
                    m_possibleValues[i].List().begin(),
                    m_possibleValues[i].List().end(),
                    [&digits]( auto d )
                    {
                        return std::find(digits.begin(),digits.end(),d) != digits.end();
                    }
                ) == m_possibleValues[i].List().end()
            )
            {
                bestValueIndexes.push_back(i);
            }
        }
        if(bestValueIndexes.size() > 0)
        {
            chosen_value_offset = bestValueIndexes[m_playerProcess_cptr->GetRandom()(bestValueIndexes.size())];
        }
    }
    return chosen_value_offset;
}

int32_t TStandartStupidBrain::chooseBestGameValueOffset()
{
    int32_t chosen_value_offset = -1;
    calcPriority();
    double bestPriority = 0;
    std::vector< uint32_t > bestValueIndexes;
    for(uint32_t i = 0; i < m_possibleValues.size(); ++i)
    {
        double priority = std::accumulate(
            m_possibleValues[i].List().begin(),
            m_possibleValues[i].List().end(),
            0.0,
            [this]( auto sum, auto digit )
            {
                return sum + m_digitsPriority[digit];
            }
        );
        if(priority >= bestPriority)
        {
            if( priority > bestPriority )
            {
                bestValueIndexes.clear();
                bestPriority = priority;
            }
            bestValueIndexes.push_back(i);
        }
    }
    if(bestValueIndexes.size() > 0)
    {
        chosen_value_offset = bestValueIndexes[m_playerProcess_cptr->GetRandom()(bestValueIndexes.size())];
    }
    return chosen_value_offset;
}

void TStandartStupidBrain::calcPriority()
{
    std::vector<uint32_t> digitsFrequency(TStandartRules::Instance().NumbersCount(), 0);
    for(auto & pair : m_digitsPriority)
    {
        pair.second = 0.0;
    }
    for( auto const & historyItem : m_playerProcess_cptr->HistoryList())
    {
        uint32_t priority = historyItem.second.first + historyItem.second.second;
        for(auto digit : historyItem.first.List())
        {
            ++digitsFrequency[digit];
            m_digitsPriority[digit] += priority;
        }
    }
    for(uint8_t i = 0; i < TStandartRules::Instance().NumbersCount(); ++i)
    {
        m_digitsPriority[i] = digitsFrequency[i] > 0
            ? m_digitsPriority[i] / (digitsFrequency[i] * digitsFrequency[i])
            : std::numeric_limits<uint32_t>::max() / TStandartRules::Instance().ValueSize();
        ;
    }

}

TStandartSmartBrain::TStandartSmartBrain( TStandartPlayerProcess const * _playerProcess )
    : TStandartStupidBrain(_playerProcess)
{
}

void TStandartSmartBrain::Init()
{
    TAnaliticBrain::Init();
}

void TStandartSmartBrain::makePredict()
{
    assert(m_possibleValues.size() > 0);
    int32_t chosen_value_offset = -1;
    if(
        m_playerProcess_cptr->HistoryList().size() > 1 ||
        (
            m_playerProcess_cptr->HistoryList().size() == 1 &&
            (
                m_playerProcess_cptr->HistoryList().back().second.first +
                m_playerProcess_cptr->HistoryList().back().second.second !=
                TStandartRules::Instance().ValueSize() - 1
            )
        )
    )
    {
        handleValuesByHistory( );
    }
    else //collect information
    {
        chosen_value_offset = chooseFirstAndSecondGameValueIndex();
    }

    if(chosen_value_offset < 0)
    {
        chosen_value_offset = chooseBestGameValueOffset();
    }
    assert(chosen_value_offset >= 0);
    m_predictedValue = std::make_shared<TGameValue<uint8_t>>(m_possibleValues[chosen_value_offset]);
    m_possibleValues.erase( m_possibleValues.begin() + chosen_value_offset );
}

void TStandartSmartBrain::restoreFromHistory()
{
    Init();
    handleValuesByHistory();
}

int32_t TStandartSmartBrain::chooseBestGameValueOffset()
{
    static std::vector<std::vector<uint32_t>> digitsCountInValues(
        TStandartRules::Instance().ValueSize(),
        std::vector<uint32_t>(TStandartRules::Instance().NumbersCount(), 0)
        );
    for(uint32_t i = 0; i < TStandartRules::Instance().ValueSize(); ++i)
    {
        std::fill(digitsCountInValues[i].begin(), digitsCountInValues[i].end(), 0);
    }
    for(const auto& value : m_possibleValues)
    {
        for(uint32_t posNo = 0; posNo < value.List().size(); ++posNo)
        {
            ++digitsCountInValues[posNo][value.List()[posNo]];
        }
    }

    std::priority_queue<
        std::tuple<uint32_t, uint8_t, uint32_t>,
        std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>,
        std::less<std::tuple<uint32_t, uint8_t, uint32_t>>
        > sortedDigitsQueue;
    for(uint32_t posNo = 0; posNo < TStandartRules::Instance().ValueSize(); ++posNo)
    {
        for(uint8_t d = 0; d < digitsCountInValues[posNo].size(); ++d)
        {
            sortedDigitsQueue.push(std::make_tuple(digitsCountInValues[posNo][d], posNo, d));
        }
    }

    int32_t chosen_value_offset = -1;
    std::vector<uint8_t> chosenGameValue(TStandartRules::Instance().ValueSize(), TStandartRules::Instance().NumbersCount());
    std::list<uint32_t> availableIndexList(m_possibleValues.size(), 0);
    std::iota(availableIndexList.begin(), availableIndexList.end(), 0);
    while(sortedDigitsQueue.size() > 0)
    {
        auto digitsTuple = sortedDigitsQueue.top();
        sortedDigitsQueue.pop();
        auto pos = std::get<1>(digitsTuple);
        auto d = std::get<2>(digitsTuple);
        if(chosenGameValue[pos] < TStandartRules::Instance().NumbersCount())
        {
            continue;
        }
        chosenGameValue[pos] = d;


        chosen_value_offset = availableIndexList.front();
        for(auto it = availableIndexList.begin(); it != availableIndexList.end();)
        {
            if(m_possibleValues[(*it)].List()[pos] != d)
            {
                it = availableIndexList.erase(it);
            }
            else
            {
                ++it;
            }
        }
        if(
            availableIndexList.size() == 0 ||
            !std::any_of(chosenGameValue.begin(), chosenGameValue.end(), [](auto d){return d == TStandartRules::Instance().NumbersCount();})
        )
        {
            break;
        }
    }
    if(availableIndexList.size() > 0)
    {
        auto rand_it = availableIndexList.begin();
        std::advance(rand_it, m_playerProcess_cptr->GetRandom()(availableIndexList.size()));
        chosen_value_offset = *rand_it;
    }

    if(chosen_value_offset < 0)
    {
        chosen_value_offset = m_playerProcess_cptr->GetRandom()(m_possibleValues.size());
    }

    return chosen_value_offset;
}

void TStandartSmartBrain::handleValuesByHistory()
{
    auto it = std::remove_if(
        m_possibleValues.begin(),
        m_possibleValues.end(),
        [this](auto const & possibleValue)
        {
            for(auto const & [gameValue, bc] : m_playerProcess_cptr->HistoryList())
            {
                auto bullsNCows = TStandartRules::Instance().calculateBullsAndCows(gameValue, possibleValue);
                if(bullsNCows.first != bc.first || bullsNCows.second != bc.second)
                {
                    return true;
                }
            }
            return false;
        }
    );
    m_possibleValues.erase(it, m_possibleValues.end());
}

std::shared_ptr<TStandartBrain> createStandartBrain( TStandartPlayerProcess const * _playerProcess, MODEL_COMPONENTS::TGameBrain _gameBrain )
{
    std::shared_ptr<TStandartBrain> gameBrain = nullptr;
    switch (_gameBrain) {
    case MODEL_COMPONENTS::TGameBrain::RANDOM:
        gameBrain = std::make_shared<TStandartRandomBrain>(_playerProcess);
        break;
    case MODEL_COMPONENTS::TGameBrain::STUPID:
        gameBrain = std::make_shared<TStandartStupidBrain>(_playerProcess);
        break;
    case MODEL_COMPONENTS::TGameBrain::SMART:
        gameBrain = std::make_shared<TStandartSmartBrain>(_playerProcess);
        break;
    case MODEL_COMPONENTS::TGameBrain::BEST:
        assert(MainNode());
        gameBrain = std::make_shared<TDecisionTreeBrain>(_playerProcess);
        break;
    default:
        break;
    }
    return gameBrain;
}
