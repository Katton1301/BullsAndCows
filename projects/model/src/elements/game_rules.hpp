#pragma once
#include "game_value.hpp"
#include <type_traits>
#include <set>
#include <memory>
#include <cassert>
#include <ctime>
#include <functional>

class TStandartRules
{
public:
    /// destructor
    ~TStandartRules( );

    static TStandartRules * Instance()
    {
        if(m_standartRules == nullptr)
        {
            m_standartRules = new TStandartRules();
        }
        return m_standartRules;
    }

    constexpr uint32_t ValueSize( )
    {
        return 4;
    }

    constexpr uint8_t NumbersCount( )
    {
        return 10;
    }

    bool isValidGameValue( TGameValue< uint8_t > const & _gameValue );
    bool isValidGameValueList( std::vector< uint8_t > const & _gameValuelist );

    std::pair<uint32_t, uint32_t> calculateBullsAndCows( TGameValue<uint8_t> const & _predictedValue, TGameValue<uint8_t> const & _trueValue);

    TGameValue<uint8_t> GetRandomGameValue( std::function< uint32_t( uint32_t ) > const & randomByModulus );

    bool isWinResults( std::pair<uint32_t, uint32_t> results );

    std::vector< TGameValue<uint8_t> > const & AllPossibleGameValues();

private: //methods
    TStandartRules();
    TStandartRules( const TStandartRules& root ) = delete;
    TStandartRules& operator=(const TStandartRules&) = delete;
    void fillPossibleValuesList( );

private: //attributes
    static TStandartRules* m_standartRules;
    std::vector< TGameValue<uint8_t> > m_possibleValues;
};
