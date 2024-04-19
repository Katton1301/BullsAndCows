#include "standart_brain.hpp"
#include "../game_process.hpp"
#include <algorithm>

TStandartBrain::TStandartBrain( TStandartGameProcess const * _gameProcess )
    : m_gameProcess_cptr(_gameProcess)
    , m_possibleValues()
    , m_predictedValue(nullptr)
{
}

void TStandartBrain::copyPossibleValuesList( )
{

    m_possibleValues.clear();
    for(auto const & gameValue : TStandartRules::Instance()->AllPossibleGameValues())
    {
        m_possibleValues.push_back(gameValue);
    }
}

void TStandartBrain::Init( )
{
    copyPossibleValuesList();
}

TGameValue<uint8_t> const * TStandartBrain::PredictedValue_cptr() const
{
    return m_predictedValue;
}

TStandartRandomBrain::TStandartRandomBrain( TStandartGameProcess const * _gameProcess )
    : TStandartBrain(_gameProcess)
{
}

void TStandartRandomBrain::makePredict( )
{
    assert(m_possibleValues.size() > 0);
    auto rnd_offset = m_gameProcess_cptr->RandomByModulus()(m_possibleValues.size());
    if(m_predictedValue != nullptr)
    {
        delete m_predictedValue;
    }
    m_predictedValue = new TGameValue(m_possibleValues.at(rnd_offset));
    m_possibleValues.erase( m_possibleValues.begin() + rnd_offset );
}

TStandartStupidBrain::TStandartStupidBrain( TStandartGameProcess const * _gameProcess )
    : TStandartBrain(_gameProcess)
{
}

void TStandartStupidBrain::Init()
{
    TStandartBrain::Init();
    m_digitsPriority.clear();
    for(uint8_t i = 0; i < TStandartRules::Instance()->NumbersCount(); ++i)
    {
        m_digitsPriority.emplace(i,0);
    }
}

void TStandartStupidBrain::makePredict( )
{
    assert(m_possibleValues.size() > 0);
    if(m_gameProcess_cptr->HistoryList().size() > 0 )
    {
        handlePredictedValueByIndex( m_gameProcess_cptr->HistoryList().size() - 1 );
    }

    int32_t chosen_value_offset = -1;
    if(m_gameProcess_cptr->HistoryList().size() < 2) //collect information
    {
        chosen_value_offset = chooseFirstAndSecondGameValueIndex();
    }

    if(chosen_value_offset < 0)
    {
        chosen_value_offset = chooseBestGameValueOffset();
    }
    assert(chosen_value_offset >= 0);
    if(m_predictedValue != nullptr)
    {
        delete m_predictedValue;
    }
    m_predictedValue = new TGameValue(m_possibleValues.at(chosen_value_offset));
    m_possibleValues.erase( m_possibleValues.begin() + chosen_value_offset );
}

void TStandartStupidBrain::handlePredictedValueByIndex( uint32_t history_i )
{
    auto const & handledValue = m_gameProcess_cptr->HistoryList().at(history_i);
    if(handledValue.second.first + handledValue.second.second == 0)
    {
        eraseValuesForDigits(handledValue.first.List());
    }
    if(handledValue.second.first + handledValue.second.second == TStandartRules::Instance()->ValueSize())
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
    if(m_gameProcess_cptr->HistoryList().size() == 0) //choose first
    {
        chosen_value_offset = m_gameProcess_cptr->RandomByModulus()(m_possibleValues.size());
    }
    else
    {
        auto const & digits = m_gameProcess_cptr->HistoryList().back().first.List();
        std::vector< uint32_t > bestValueIndexes;
        for(uint32_t i = 0; i < m_possibleValues.size(); ++i)
        {
            if(
                std::ranges::find_if(
                    m_possibleValues.at(i).List().begin(),
                    m_possibleValues.at(i).List().end(),
                    [&digits]( auto d )
                    {
                        return std::find(digits.begin(),digits.end(),d) != digits.end();
                    }
                ) == m_possibleValues.at(i).List().end()
            )
            {
                bestValueIndexes.push_back(i);
            }
        }
        if(bestValueIndexes.size() > 0)
        {
            chosen_value_offset = bestValueIndexes.at(m_gameProcess_cptr->RandomByModulus()(bestValueIndexes.size()));
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
            m_possibleValues.at(i).List().begin(),
            m_possibleValues.at(i).List().end(),
            0.0,
            [this]( auto sum, auto digit )
            {
                return sum + m_digitsPriority.at(digit);
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
        chosen_value_offset = bestValueIndexes.at(m_gameProcess_cptr->RandomByModulus()(bestValueIndexes.size()));
    }
    return chosen_value_offset;
}

void TStandartStupidBrain::calcPriority()
{
    std::vector<uint32_t> digitsFrequency;
    for(uint8_t i = 0; i < TStandartRules::Instance()->NumbersCount(); ++i)
    {
        digitsFrequency.push_back(0);
        m_digitsPriority.at(i) = 0.0;
    }
    for( auto const & historyItem : m_gameProcess_cptr->HistoryList())
    {
        uint32_t priority = historyItem.second.first + historyItem.second.second;
        for(auto digit : historyItem.first.List())
        {
            ++digitsFrequency.at(digit);
            m_digitsPriority.at(digit) += priority;
        }
    }
    for(uint8_t i = 0; i < TStandartRules::Instance()->NumbersCount(); ++i)
    {
        m_digitsPriority.at(i) = digitsFrequency.at(i) > 0
            ? m_digitsPriority.at(i) / (digitsFrequency.at(i) * digitsFrequency.at(i))
            : std::numeric_limits<uint32_t>::max() / TStandartRules::Instance()->ValueSize();
        ;
    }

}

TStandartSmartBrain::TStandartSmartBrain( TStandartGameProcess const * _gameProcess )
    : TStandartStupidBrain(_gameProcess)
{
}

void TStandartSmartBrain::Init()
{
    TStandartBrain::Init();
}

void TStandartSmartBrain::makePredict()
{
    assert(m_possibleValues.size() > 0);
    int32_t chosen_value_offset = -1;
    if( m_gameProcess_cptr->HistoryList().size() > 0 )
    {
        checkValuesByHistoryList( );
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
    if(m_predictedValue != nullptr)
    {
        delete m_predictedValue;
    }
    m_predictedValue = new TGameValue(m_possibleValues.at(chosen_value_offset));
    m_possibleValues.erase( m_possibleValues.begin() + chosen_value_offset );
}

int32_t TStandartSmartBrain::chooseBestGameValueOffset()
{
    int32_t chosen_value_offset = -1;
    std::vector< std::vector< uint32_t > >
        digitsCountInValues(
            TStandartRules::Instance()->ValueSize(),
            std::vector< uint32_t >(TStandartRules::Instance()->NumbersCount(), 0)
        );

    for(uint32_t value_offset = 0; value_offset < m_possibleValues.size(); ++value_offset)
    {
        for(uint32_t posNo = 0; posNo < m_possibleValues.at(value_offset).List().size(); ++posNo)
        {
            ++digitsCountInValues.at(posNo).at(m_possibleValues.at(value_offset).List().at(posNo));
        }
    }

    std::vector< std::vector<uint8_t> > sortDigitsList;
    for(uint32_t posNo = 0; posNo < TStandartRules::Instance()->ValueSize(); ++posNo)
    {
        std::vector<uint8_t> sortedDigits;
        while(true)
        {
            std::vector<uint8_t> bestDigits{};
            uint32_t maxCount = 0;
            for(uint8_t digit = 0; digit < digitsCountInValues.at(posNo).size(); ++digit)
            {
                if(maxCount <= digitsCountInValues.at(posNo).at(digit))
                {
                    if(maxCount < digitsCountInValues.at(posNo).at(digit))
                    {
                        maxCount = digitsCountInValues.at(posNo).at(digit);
                        bestDigits.clear();
                    }
                    bestDigits.push_back(digit);
                }
            }

            if(maxCount == 0)
            {
                break;
            }
            else
            {
                while(bestDigits.size() > 0)
                {
                    std::swap(
                        bestDigits.at(m_gameProcess_cptr->RandomByModulus()(bestDigits.size())),
                        bestDigits.back()
                    );
                    digitsCountInValues.at(posNo).at(bestDigits.back()) = 0;
                    sortedDigits.push_back(bestDigits.back());
                    bestDigits.pop_back();
                }
            }
        }
        sortDigitsList.push_back(sortedDigits);
    }

    int32_t posNo = 0;
    std::vector<uint32_t> digitsNumbersList(TStandartRules::Instance()->ValueSize(), 0);
    std::vector<uint8_t> chosenGameValue;
    while(posNo > -1)
    {
        if(sortDigitsList.at(posNo).size() <= digitsNumbersList.at(posNo))
        {
            digitsNumbersList.at(posNo) = 0;
            --posNo;
            ++digitsNumbersList.at(posNo);
            chosenGameValue.pop_back();
            continue;
        }

        if(
            std::find(
                chosenGameValue.begin(),
                chosenGameValue.end(),
                sortDigitsList.at(posNo).at(digitsNumbersList.at(posNo))
            ) != chosenGameValue.end()
        )
        {
            ++digitsNumbersList.at(posNo);
            continue;
        }

        chosenGameValue.push_back(sortDigitsList.at(posNo).at(digitsNumbersList.at(posNo)));

        if(chosenGameValue.size() == TStandartRules::Instance()->ValueSize())
        {
            for(uint32_t valueOffset = 0; valueOffset < m_possibleValues.size(); ++valueOffset)
            {
                bool valueFinded = true;
                for(uint32_t i = 0; i < m_possibleValues.at(valueOffset).List().size(); ++i)
                {
                    if(m_possibleValues.at(valueOffset).List().at(i) != chosenGameValue.at(i))
                    {
                        valueFinded = false;
                        break;
                    }
                }
                if(valueFinded)
                {
                    chosen_value_offset = valueOffset;
                }
            }
            if(chosen_value_offset > -1)
            {
                break;
            }
            ++digitsNumbersList.at(posNo);
            chosenGameValue.pop_back();
        }
        else
        {
            ++posNo;
        }

    }

    if(chosen_value_offset < 0)
    {
        chosen_value_offset = m_gameProcess_cptr->RandomByModulus()(m_possibleValues.size());
    }

    return chosen_value_offset;
}

void TStandartSmartBrain::checkValuesByHistoryList()
{
    auto it = m_possibleValues.begin();
    while( it != m_possibleValues.end() )
    {
        bool validValue = true;
        for(auto const & historyItem : m_gameProcess_cptr->HistoryList())
        {
            auto bullsNCows = TStandartRules::Instance()->calculateBullsAndCows(historyItem.first, *it);
            if(bullsNCows.first != historyItem.second.first || bullsNCows.second != historyItem.second.second)
            {
                validValue = false;
                break;
            }
        }
        if(!validValue)
        {
            it = m_possibleValues.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::shared_ptr<TStandartBrain> createStandartBrain( TStandartGameProcess const * _gameProcess, MODEL_COMPONENTS::TGameBrain _gameBrain )
{
    std::shared_ptr<TStandartBrain> gameBrain = nullptr;
    switch (_gameBrain) {
    case MODEL_COMPONENTS::TGameBrain::RANDOM:
        gameBrain = std::make_shared<TStandartRandomBrain>(_gameProcess);
        break;
    case MODEL_COMPONENTS::TGameBrain::STUPID:
        gameBrain = std::make_shared<TStandartStupidBrain>(_gameProcess);
        break;
    case MODEL_COMPONENTS::TGameBrain::SMART:
        gameBrain = std::make_shared<TStandartSmartBrain>(_gameProcess);
        break;
    default:
        gameBrain = std::make_shared<TStandartRandomBrain>(_gameProcess);
        break;
    }
    return gameBrain;
}
