#pragma once
#include <cstdint>
#include <map>
#include "standart_rules.hpp"
#include <rapidjson/document.h>

class TValueNode
{
public:
    using TGameValueList = TStandartRules::TGameValueList;
    using TChildNodes = std::map< std::pair<uint32_t, uint32_t>, std::shared_ptr<TValueNode> >;

public:
    TValueNode() = delete;
    TValueNode(TGameValueList const & _value, double _steps, uint32_t _weight, uint32_t _depth )
    : value(_value)
    , steps(_steps)
    , weight(_weight)
    , depth(_depth)
    {}

    ~TValueNode()
    {
    }

    double Steps() const
    {
        return steps;
    }

    std::vector<uint8_t> const & Value() const
    {
        return value;
    }

    uint32_t Weight() const
    {
        return weight;
    }

    uint32_t Depth() const
    {
        return depth;
    }

    std::shared_ptr<TValueNode> const & ChildsAt(int32_t bulls, uint32_t cows) const
    {
        if(!childs.contains({bulls,cows}))
        {
            std::cout << "stop" << std::endl;
        }
        return childs.at({bulls,cows});
    }

    bool ContainBullsNCows(uint32_t bulls, uint32_t cows) const
    {
        return childs.contains({bulls,cows});
    }

    TChildNodes const & Childs() const
    {
        return childs;
    }

    void addChild( uint32_t _bulls, uint32_t _cows, std::shared_ptr<TValueNode> && _child);

    void recalcSteps();
    void updateWeight();

private:
    TGameValueList value;
    double steps;
    uint32_t weight;
    uint32_t depth;
    TChildNodes childs{};
};

std::ostream& tabs(std::ostream& _stream_, uint32_t N );

std::ostream& operator<<( std::ostream& _stream_, TValueNode const & _parentNode);

namespace JSON_TOOLS
{
    void writeToJsonStorage( rapidjson::Document * storage, std::shared_ptr<TValueNode> const & _parentNode, rapidjson::Document * external_storage );
    void writeToJson( std::string const & _path, std::shared_ptr<TValueNode> const & _parentNode );
    std::shared_ptr<TValueNode> loadNodeFromJsonStorage( const rapidjson::Value * storage );
    std::shared_ptr<TValueNode > loadNodeFromJson( std::string const & _path, std::vector<uint8_t> mainValue );
}

