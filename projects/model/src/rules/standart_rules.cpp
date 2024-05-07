#include "standart_rules.hpp"

TStandartRules* TStandartRules::m_standartRules = nullptr;


TStandartRules::TStandartRules()
{
    fillPossibleValuesList();
}

TStandartRules::~TStandartRules( )
{
    if ( m_standartRules != nullptr )
    {
        TStandartRules* standartRules = m_standartRules;
        m_standartRules = nullptr;
        delete standartRules;
    }
}

bool TStandartRules::isValidGameValue( TGameValue< uint8_t > const & _gameValue )
{
    return isValidGameValueList(_gameValue.List());
}

bool TStandartRules::isValidGameValueList( TGameValueList const & _gameValuelist )
{
    std::set<uint8_t> uniqList;
    if(_gameValuelist.size() != ValueSize())
    {
        return false;
    }
    for( auto digit : _gameValuelist )
    {
        if(digit >= NumbersCount() || uniqList.contains(digit))
        {
            return false;
        }
        uniqList.emplace(digit);
    }
    return true;
}

std::pair<uint32_t, uint32_t> TStandartRules::calculateBullsAndCows( TGameValue<uint8_t> const & _predictedValue, TGameValue<uint8_t> const & _trueValue)
{
    return calculateBullsAndCows(_predictedValue.List(), _trueValue.List() );
}

std::pair<uint32_t, uint32_t> TStandartRules::calculateBullsAndCows( TGameValueList const & _predictedValue, TGameValueList const & _trueValue)
{
    uint32_t bulls = 0;
    uint32_t cows = 0;
    for(uint32_t i = 0; i < ValueSize( ); ++i)
    {
        for(uint32_t j = 0; j < ValueSize( ); ++j)
        {
            if(_predictedValue.at(i) == _trueValue.at(j))
            {
                if(i == j)
                {
                    ++bulls;
                }
                else
                {
                    ++cows;
                }
                break;
            }
        }
    }
    return std::make_pair(bulls,cows);
}

TGameValue<uint8_t> TStandartRules::GetRandomGameValue( std::function< uint32_t( uint32_t ) > const & randomByModulus )
{
    TGameValueList uniqList;
    for(uint8_t i = 0; i < NumbersCount(); ++i)
    {
        uniqList.push_back(i);
    }
    std::vector<uint8_t> gameValueList;
    for(uint32_t i = 0; i < ValueSize( ); ++i)
    {
       uint32_t randPos = randomByModulus( uniqList.size() );
       std::swap(uniqList.at(randPos), uniqList.back());
       gameValueList.push_back(uniqList.back());
       uniqList.pop_back();
    }
    return TGameValue(gameValueList);
}

bool TStandartRules::isWinResults( std::pair<uint32_t, uint32_t> results )
{
    return results.first == ValueSize() && results.second == 0;
}

void TStandartRules::fillPossibleValuesList( )
{
    TGameValueList possibleValue(ValueSize(), 0);
    int32_t pos = 0;
    while( pos >= 0 )
    {
        if(isValidGameValueList(possibleValue))
        {
            m_possibleValues.push_back(TGameValue(possibleValue));
        }

        pos = ValueSize() - 1;
        ++possibleValue.at(pos);

        while(possibleValue.at(pos) == NumbersCount())
        {
            possibleValue.at(pos) = 0;
            --pos;
            if(pos < 0)
            {
                break;
            }
            ++possibleValue.at(pos);
        }
    }
}

std::vector< TGameValue<uint8_t> > const & TStandartRules::AllPossibleGameValues()
{
    return m_possibleValues;
}
