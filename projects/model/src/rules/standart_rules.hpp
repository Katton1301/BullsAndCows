#pragma once
#include <core/game_value.hpp>
#include <type_traits>
#include <set>
#include <memory>
#include <cassert>
#include <ctime>
#include <functional>

class TStandartRules
{
public:
    using TGameValueList = std::vector<uint8_t>;
    /// destructor
    ~TStandartRules( );

    static TStandartRules & Instance()
    {
        static TStandartRules standartRules;
        return standartRules;
    }

    constexpr uint32_t ValueSize( )
    {
        return 4;
    }

    constexpr uint8_t NumbersCount( )
    {
        return 10;
    }

    constexpr uint32_t AllValuesNumber( )
    {
        uint32_t v = NumbersCount();
        uint32_t n = 1;
        for(uint32_t i = 0; i < ValueSize();  ++i)
        {
            n *= v--;
        }
        return n;
    }

    bool isValidGameValue( TGameValue< uint8_t > const & _gameValue );
    bool isValidGameValueList( TGameValueList const & _gameValuelist );

    std::pair<uint32_t, uint32_t> calculateBullsAndCows( TGameValue<uint8_t> const & _predictedValue, TGameValue<uint8_t> const & _trueValue);
    std::pair<uint32_t, uint32_t> calculateBullsAndCows( TGameValueList const & _predictedValue, TGameValueList const & _trueValue);

    TGameValue<uint8_t> GetRandomGameValue( std::function< uint32_t( uint32_t ) > const & randomByModulus );

    inline bool isWinResults( std::pair<uint32_t, uint32_t> results )
    {
        return results.first == ValueSize() && results.second == 0;
    }

    uint32_t gameValueToUint( TGameValueList const & gameValue ) const;
    std::string gameValueToString( TGameValueList const & gameValue );

    std::vector< TGameValue<uint8_t> > const & AllPossibleGameValues() const;

private: //methods
    TStandartRules();
    TStandartRules( const TStandartRules& root ) = delete;
    TStandartRules& operator=(const TStandartRules&) = delete;
    void fillPossibleValuesList( );

private: //attributes
    std::vector< TGameValue<uint8_t> > m_possibleValues;
};
