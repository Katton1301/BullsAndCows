#include <rules/standart_rules.hpp>
#include<numeric>

TStandartRules::TStandartRules()
{
    fillPossibleValuesList();
}

TStandartRules::~TStandartRules( )
{
}

bool TStandartRules::isValidGameValue( TGameValue< uint8_t > const & _gameValue )
{
    return isValidGameValueList(_gameValue.List());
}

bool TStandartRules::isValidGameValueList( TGameValueList const & _gameValuelist )
{
    if(_gameValuelist.size() != ValueSize())
    {
        return false;
    }
    std::set<uint8_t> uniqList;
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

uint32_t TStandartRules::gameValueToUint( TGameValueList const & gameValue ) const
{
    uint32_t n = 0;
    for(auto digit : gameValue)
    {
        n = n * 10 + static_cast<uint32_t>(digit);
    }
    return n;
}

std::string TStandartRules::gameValueToString( TGameValueList const & gameValue )
{
    std::string s;
    s.resize(ValueSize());
    for(uint32_t i = 0; i < ValueSize(); ++i)
    {
        s[i] = static_cast<uint32_t>(gameValue[i]);
    }
    return s;
}

std::pair<uint32_t, uint32_t> TStandartRules::calculateBullsAndCows( TGameValue<uint8_t> const & _predictedValue, TGameValue<uint8_t> const & _trueValue)
{
    return calculateBullsAndCows(_predictedValue.List(), _trueValue.List() );
}

std::pair<uint32_t, uint32_t> TStandartRules::calculateBullsAndCows( TGameValueList const & _predictedValue, TGameValueList const & _trueValue)
{
    uint32_t bulls = 0;
    uint32_t cows = 0;
    uint16_t mask = 0;
    for(uint32_t i = 0; i < ValueSize( ); ++i)
    {
        if(_predictedValue[i] == _trueValue[i])
        {
            ++bulls;
        }
        else
        {
            mask |= 1 << _trueValue[i];
        }
    }
    for(uint32_t i = 0; i < ValueSize( ); ++i)
    {
        if(_predictedValue[i] != _trueValue[i] && (mask & (1 << _predictedValue[i])))
        {
            ++cows;
        }
    }
    return {bulls,cows};
}

TGameValue<uint8_t> TStandartRules::GetRandomGameValue( std::function< uint32_t( uint32_t ) > const & randomByModulus )
{
    static TGameValueList uniqList(NumbersCount(),0);
    std::iota(uniqList.begin(),uniqList.end(), 0);
    std::vector<uint8_t> gameValueList;
    gameValueList.reserve(ValueSize());
    uint32_t randOffset = randomByModulus(AllValuesNumber());
    uint32_t randPos;
    for(uint32_t i = 0; i < ValueSize( ); ++i)
    {
        randPos = randOffset % (NumbersCount() - i);
        randOffset /= (NumbersCount() - i);
        gameValueList.push_back(uniqList[randPos]);
        std::swap(uniqList[randPos], uniqList[NumbersCount() - i - 1]);
    }
    return TGameValue(gameValueList);
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

std::vector< TGameValue<uint8_t> > const & TStandartRules::AllPossibleGameValues() const
{
    return m_possibleValues;
}
