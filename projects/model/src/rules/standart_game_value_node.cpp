#include <rules/standart_game_value_node.hpp>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

void TValueNode::addChild( uint32_t _bulls, uint32_t _cows, std::shared_ptr<TValueNode> && _child)
{
    if(
        !childs.contains({_bulls,_cows}) ||
        childs.at({_bulls, _cows})->Steps() >= _child->Steps()
    )
    {
        childs[{_bulls, _cows}] = _child;
    }
}

void TValueNode::recalcSteps()
{
    uint32_t weightSumm = 0;
    double stepsSum = 0.0;
    for(auto const & [bc, node] : childs)
    {
        stepsSum += node->Steps() * node->Weight();
        weightSumm += node->Weight();
    }
    if(weightSumm != 0)
    {
        steps = 1.0 + (stepsSum / weightSumm);
    }
}

void TValueNode::updateWeight()
{
    if(childs.size() == 0)
    {
        weight = 1;
        return;
    }
    uint32_t newWeight = 0;
    for(auto & [bc, node] : childs)
    {
        node->updateWeight();
        newWeight += node->Weight();
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
    for( auto const & [bc, node] : _parentNode.Childs() )
    {
        tabs(_stream_, tabCount);
        _stream_ << bc.first << " B " << bc.second << " C " << std::endl;
        tabs(_stream_, tabCount);
        _stream_ << "Child:" << std::endl;
        _stream_ << node;
    }
    return _stream_;
}



namespace JSON_TOOLS
{
    void writeToJsonStorage( rapidjson::Document * storage, std::shared_ptr<TValueNode> const & _parentNode, rapidjson::Document * external_storage )
    {
        using namespace rapidjson;
        uint32_t gameValue = TStandartRules::Instance().gameValueToUint(_parentNode->Value());
        storage->AddMember("V", Value().SetUint(gameValue), external_storage->GetAllocator());
        storage->AddMember("S", Value().SetDouble(_parentNode->Steps()), external_storage->GetAllocator());
        storage->AddMember("A", Value().SetUint(_parentNode->Depth()), external_storage->GetAllocator());
        Value bc_container(kArrayType);
        for (auto const & [bc, child] : _parentNode->Childs())
        {
            Document bc_storage( kObjectType );
            bc_storage.AddMember("BC", Value().SetUint(bc.first * 10 + bc.second), external_storage->GetAllocator());
            Document child_storage( kObjectType );
            writeToJsonStorage(&child_storage, child, external_storage);
            bc_storage.AddMember("C", child_storage, external_storage->GetAllocator());
            bc_container.PushBack(bc_storage, external_storage->GetAllocator());
        }
        storage->AddMember("Var", bc_container, external_storage->GetAllocator());
    }

    std::shared_ptr<TValueNode> loadNodeFromJsonStorage( const rapidjson::Value * storage )
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
        std::shared_ptr<TValueNode> parentNode = std::make_shared<TValueNode>(vectorValue, steps, 1, depth);
        for (auto const & child_storage : (*storage)["Var"].GetArray())
        {
            if (
                !child_storage.IsObject() ||
                !child_storage.HasMember("BC") ||
                !child_storage.HasMember("C") ||
                !child_storage["C"].IsObject()
            )
            {
                continue;
            }
            uint32_t bulls = child_storage["BC"].GetUint() / 10;
            uint32_t cows = child_storage["BC"].GetUint() % 10;

            std::shared_ptr<TValueNode> childNode = loadNodeFromJsonStorage(&child_storage["C"]);
            if(childNode)
            {
                parentNode->addChild(bulls, cows, std::move(childNode));
            }
        }

        return parentNode;
    }

    void writeToJson( std::string const & _json_path, std::shared_ptr<TValueNode> const & _parentNode )
    {
        using namespace rapidjson;
        Document external_storage( kObjectType );
        writeToJsonStorage(&external_storage, _parentNode, &external_storage);

        std::ofstream storage_file(_json_path.c_str(), std::ios::trunc);
        StringBuffer buffer;
        PrettyWriter < rapidjson::StringBuffer > writer( buffer );
        writer.SetIndent( ' ', 1 );
        writer.SetFormatOptions( rapidjson::kFormatSingleLineArray );
        external_storage.Accept(writer);
        writer.Flush();

        std::string export_storage = buffer.GetString();
        std::ranges::copy(export_storage.begin(), export_storage.end(), std::ostream_iterator<char>(storage_file,""));
    }

    std::shared_ptr<TValueNode > loadNodeFromJson( std::string const & _json_path, std::vector<uint8_t> mainValue )
    {
        std::ifstream fileToOpen( _json_path.c_str( ) );
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

        if( TStandartRules::Instance().gameValueToUint(mainValue) != value )
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
        std::shared_ptr<TValueNode > parentNode = std::make_shared<TValueNode>(vectorValue, steps, 1, depth);
        for (auto const & child_storage : json_storage["Var"].GetArray())
        {
            if (
                !child_storage.IsObject() ||
                !child_storage.HasMember("BC") ||
                !child_storage.HasMember("C") ||
                !child_storage["C"].IsObject()
            )
            {
                continue;
            }
            uint32_t bulls = child_storage["BC"].GetUint() / 10;
            uint32_t cows = child_storage["BC"].GetUint() % 10;
            std::shared_ptr<TValueNode> childNode = loadNodeFromJsonStorage(&child_storage["C"]);
            if(childNode)
            {
                parentNode->addChild(bulls, cows, std::move(childNode));
            }
        }
        parentNode->updateWeight();
        return parentNode;
    }
}
