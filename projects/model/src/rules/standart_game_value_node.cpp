#include "standart_game_value_node.hpp"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

void TValueNode::addChild( uint32_t _bulls, uint32_t _cows, TValueNode *_child)
{
    if(!childs.contains({_bulls,_cows}))
    {
        childs.emplace( std::make_pair(_bulls, _cows), std::vector<TValueNode *>());
    }
    if(
        childs.at({_bulls, _cows}).size() == 0 ||
        childs.at({_bulls, _cows}).front()->Steps() >= _child->Steps()
    )
    {
        if(
            childs.at({_bulls, _cows}).size() > 0 &&
            childs.at({_bulls, _cows}).front()->Steps() > _child->Steps()
        )
        {
            for(auto const & child : childs.at({_bulls, _cows}))
            {
                delete child;
            }
        }
        childs.at({_bulls, _cows}).push_back(_child);
    }
}

void TValueNode::recalcSteps()
{
    uint32_t weightSumm = 0;
    double stepsSum = 0.0;
    for(auto const & child : childs)
    {
        stepsSum += child.second.front()->Steps() * child.second.front()->Weight();
        weightSumm += child.second.front()->Weight();
    }
    steps = 1.0 + (stepsSum / weightSumm);
}

void TValueNode::updateWeight()
{
    if(childs.size() == 0)
    {
        weight = 1;
        return;
    }
    uint32_t newWeight = 0;
    for(auto & child : childs)
    {
        child.second.front()->updateWeight();
        newWeight += child.second.front()->Weight();
    }
    weight = newWeight;
}

std::ostream& tabs(std::ostream& _stream_, uint32_t N )
{
    for(uint32_t i = 0; i < N; ++i)
    {
        _stream_ << "\t";
    }
    return _stream_;
}

std::ostream& operator<<( std::ostream& _stream_, TValueNode const & _parentNode)
{
    uint32_t tabCount = _parentNode.Depth() - 1;
    tabs(_stream_, tabCount);
    _stream_ << static_cast<uint32_t>(_parentNode.Value().at(0))
             << static_cast<uint32_t>(_parentNode.Value().at(1))
             << static_cast<uint32_t>(_parentNode.Value().at(2))
             << static_cast<uint32_t>(_parentNode.Value().at(3))
             << std::endl;
    tabs(_stream_, tabCount);
    _stream_ << "avg steps " << _parentNode.Steps() << std::endl;
    for( auto const & child : _parentNode.Childs() )
    {
        tabs(_stream_, tabCount);
        _stream_ << child.first.first << " B " << child.first.second << " C " << child.second.size() << " values" << std::endl;
        tabs(_stream_, tabCount);
        _stream_ << "Childs:" << std::endl;
        for(auto const & childOfChild : child.second)
        {
            _stream_ << *childOfChild;
        }
    }
    return _stream_;
}

void writeToJson( rapidjson::Document * storage, TValueNode const & _parentNode, rapidjson::Document * external_storage )
{
    using namespace rapidjson;
    uint32_t gameValue =
            _parentNode.Value().at(0) * 1000 +
            _parentNode.Value().at(1) * 100 +
            _parentNode.Value().at(2) * 10 +
            _parentNode.Value().at(3)
        ;
    storage->AddMember("V", Value().SetUint(gameValue), external_storage->GetAllocator());
    storage->AddMember("S", Value().SetDouble(_parentNode.Steps()), external_storage->GetAllocator());
    storage->AddMember("A", Value().SetUint(_parentNode.Depth()), external_storage->GetAllocator());
    Value bc_container(kArrayType);
    for (auto const & [bc, childs] : _parentNode.Childs())
    {
        Document bc_storage( kObjectType );
        bc_storage.AddMember("BC", Value().SetUint(bc.first * 10 + bc.second), external_storage->GetAllocator());
        Value childs_container(kArrayType);
        for(auto const & childNode : childs)
        {
            Document child_storage( kObjectType );
            writeToJson(&child_storage, *childNode, external_storage);
            childs_container.PushBack(child_storage, external_storage->GetAllocator());
        }
        bc_storage.AddMember("C", childs_container, external_storage->GetAllocator());
        bc_container.PushBack(bc_storage, external_storage->GetAllocator());
    }
    storage->AddMember("Var", bc_container, external_storage->GetAllocator());
}

void writeToJson( std::string const & _path, TValueNode const & _parentNode )
{
    std::string strPathName = _path + "bc.json";
    using namespace rapidjson;
    Document external_storage( kObjectType );
    writeToJson(&external_storage, _parentNode, &external_storage);

    std::ofstream storage_file(strPathName.c_str(), std::ios::trunc);
    StringBuffer buffer;
    PrettyWriter < rapidjson::StringBuffer > writer( buffer );
    writer.SetIndent( ' ', 1 );
    writer.SetFormatOptions( rapidjson::kFormatSingleLineArray );
    external_storage.Accept(writer);
    writer.Flush();

    std::string export_storage = buffer.GetString();
    std::ranges::copy(export_storage.begin(), export_storage.end(), std::ostream_iterator<char>(storage_file,""));
}

TValueNode * loadNodeFromJson( const rapidjson::Value * storage )
{
    if (
        !storage->IsObject() ||
        !storage->HasMember("V") ||
        !storage->HasMember("S") ||
        !storage->HasMember("A") ||
        !storage->HasMember("Var") ||
        !(*storage)["Var"].IsArray()
    )
    {
        return nullptr;
    }

    uint32_t value = (*storage)["V"].GetUint();
    TValueNode::TGameValueList vectorValue;
    while(vectorValue.size() < 4)
    {
        vectorValue.insert(vectorValue.begin(), value % 10);
        value /= 10;
    }
    double steps = (*storage)["S"].GetDouble();
    uint32_t depth = (*storage)["A"].GetUint();
    TValueNode * parentNode = new TValueNode(vectorValue, steps, 1, depth);
    for (auto const & child_storage : (*storage)["Var"].GetArray())
    {
        if (
            !child_storage.IsObject() ||
            !child_storage.HasMember("BC") ||
            !child_storage.HasMember("C") ||
            !child_storage["C"].IsArray()
        )
        {
            continue;
        }
        uint32_t bulls = child_storage["BC"].GetUint() / 10;
        uint32_t cows = child_storage["BC"].GetUint() % 10;
        for (auto const & bc_storage : child_storage["C"].GetArray())
        {
            TValueNode * childNode = loadNodeFromJson(&bc_storage);
            if(childNode)
            {
                parentNode->addChild(bulls, cows, childNode);
            }
        }
    }

    return parentNode;
}

TValueNode * loadNodeFromJson( std::string const & _path, std::vector<uint8_t> mainValue )
{
    std::string strPathName = _path + "bc.json";
    std::ifstream fileToOpen( strPathName.c_str( ) );
    if ( !fileToOpen.is_open( ) )
    {
        return nullptr;
    }
    rapidjson::IStreamWrapper json_wrapper(fileToOpen);

    rapidjson::Document json_storage{};
    json_storage.ParseStream(json_wrapper);

    if (
        !json_storage.IsObject() ||
        !json_storage.HasMember("V") ||
        !json_storage.HasMember("S") ||
        !json_storage.HasMember("A") ||
        !json_storage.HasMember("Var") ||
        !json_storage["Var"].IsArray()
    )
    {
        return nullptr;
    }

    uint32_t value = json_storage["V"].GetUint();

    if(
        static_cast<uint32_t>(mainValue.at(0)) * 1000 +
        static_cast<uint32_t>(mainValue.at(1)) * 100 +
        static_cast<uint32_t>(mainValue.at(2)) * 10 +
        static_cast<uint32_t>(mainValue.at(3))
        != value
    )
    {
        return nullptr;
    }

    TValueNode::TGameValueList vectorValue;
    while(vectorValue.size() < 4)
    {
        vectorValue.insert(vectorValue.begin(), value % 10);
        value /= 10;
    }
    double steps = json_storage["S"].GetDouble();
    uint32_t depth = json_storage["A"].GetUint();
    TValueNode * parentNode = new TValueNode(vectorValue, steps, 1, depth);
    for (auto const & child_storage : json_storage["Var"].GetArray())
    {
        if (
            !child_storage.IsObject() ||
            !child_storage.HasMember("BC") ||
            !child_storage.HasMember("C") ||
            !child_storage["C"].IsArray()
        )
        {
            continue;
        }
        uint32_t bulls = child_storage["BC"].GetUint() / 10;
        uint32_t cows = child_storage["BC"].GetUint() % 10;
        for (auto const & bc_storage : child_storage["C"].GetArray())
        {
            TValueNode * childNode = loadNodeFromJson(&bc_storage);
            if(childNode)
            {
                parentNode->addChild(bulls, cows, childNode);
            }
        }
    }
    parentNode->updateWeight();
    return parentNode;
}
