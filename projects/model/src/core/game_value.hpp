#pragma once
#include<vector>
#include<iostream>

template< typename TValue >
class TGameValue
{
public: //attributes
    using TGameValueList = std::vector< TValue >;

public: //methods
    TGameValue( ) = delete;
    TGameValue( std::vector< TValue > const & _valueList )
        : m_valueList(_valueList)
    {
    }

    TGameValue( TGameValue< TValue > const & _gameValue )
    {
        setList(_gameValue.List());
    }

    TGameValue & operator=( const TGameValue< TValue > & _gameValue )
    {
        if ( &_gameValue == this )
        {
            return *this;
        }
        setList(_gameValue.List());
        return *this;
    }

    ~TGameValue( )
    {

    }

    TGameValueList const & List() const
    {
        return m_valueList;
    }



private: //methods
    void setList(std::vector< TValue > const & _valueList)
    {
        m_valueList = _valueList;
    }
private: //attributes

    TGameValueList m_valueList;
};

template< typename TValue >
std::ostream& operator<<( std::ostream& stream_, const TGameValue< TValue >& _gameValue )
{
    stream_ << "Value List" << std::endl;
    for(auto & digit : _gameValue.List())
    {
        std::cout << digit << " ";
    }
    stream_ << std::endl;
    return stream_;
}
