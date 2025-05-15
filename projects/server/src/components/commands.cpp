#include <components/commands.hpp>

namespace SERVER_COMPONENTS
{
    TRequestData ParseKafkaMessage( std::string message )
    {
        TRequestData request;
        rapidjson::Document doc;
        doc.Parse(message.c_str());

        if (doc.HasParseError())
        {
            return request;
        }

        if (!doc.IsObject())
        {
            return request;
        }

        const char* requiredFields[] = {"server_id", "command", "correlation_id"};
        for (const char* field : requiredFields)
        {
            if (!doc.HasMember(field))
            {
                return request;
            }
        }

        request.ServerId = doc["server_id"].GetUint();
        request.CorrelationId = doc["correlation_id"].GetString();
        request.Command = static_cast<TCommand>(doc["command"].GetUint());
        if (doc.HasMember("player_id") && doc["player_id"].IsUint())
        {
            request.PlayerId = doc["player_id"].GetUint();
        }
        if (doc.HasMember("game_id") && doc["game_id"].IsUint())
        {
            request.GameId = doc["game_id"].GetUint();
        }
        if (doc.HasMember("game_brain") && doc["game_brain"].IsString())
        {
            request.GameBrain = COMMON_OPERATIONS::LevelNameToGameBrain(doc["game_brain"].GetString());
        }
        if (doc.HasMember("game_value"))
        {
            const rapidjson::Value& gameValueJson = doc["game_value"];
            std::vector<uint8_t> gameValue;
            if (gameValueJson.IsString())
            {
                std::string gameString = gameValueJson.GetString();
                for(uint32_t i = 0; TStandartRules::Instance().ValueSize(); ++i)
                {
                    gameValue.push_back(gameString[i] - '0');
                }
            }
            else if (gameValueJson.IsArray())
            {
                for (const auto& item : gameValueJson.GetArray())
                {
                    gameValue.push_back(static_cast<uint8_t>(item.GetUint()));
                }
            }
            else if (gameValueJson.IsUint())
            {
                uint32_t gameValueInt = gameValueJson.GetUint();
                gameValue.resize(TStandartRules::Instance().ValueSize());
                for(int32_t i = TStandartRules::Instance().ValueSize() - 1; i >= 0; --i)
                {
                    gameValue[i] = gameValueInt % 10;
                    gameValueInt /= 10;
                }
            }
            request.GameValue = gameValue;
        }
        if (doc.HasMember("computer_id") && doc["computer_id"].IsUint())
        {
            request.ComputerId = doc["computer_id"].GetUint();
        }
        if (doc.HasMember("step") && doc["step"].IsUint())
        {
            request.Step = doc["step"].GetUint();
        }

        return request;
    }

    std::string SerializeResultToKafkaMessage(TResultData const & resultData)
    {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        doc.AddMember("server_id", resultData.ServerId, allocator);
        doc.AddMember("correlation_id", rapidjson::Value().SetString(resultData.CorrelationId.c_str(), allocator), allocator);
        doc.AddMember("result", static_cast<int>(resultData.Result), allocator);
        if(resultData.GameStage != MODEL_COMPONENTS::TGameStage::UNKNOWN)
        {
            doc.AddMember("game_stage", rapidjson::Value().SetString(MODEL_COMPONENTS::gameStageToString(resultData.GameStage).c_str(), allocator), allocator);
        }
        if(resultData.GameId > 0)
        {
            doc.AddMember("game_id", resultData.GameId, allocator);
        }
        if(resultData.PlayerId > 0)
        {
            doc.AddMember("player_id", resultData.PlayerId, allocator);
        }
        if(resultData.Players > 0)
        {
            doc.AddMember("players", resultData.Players, allocator);
        }
        if(resultData.SecretValue.size() > 0)
        {
            doc.AddMember("secret_value", TStandartRules::Instance().gameValueToUint(resultData.SecretValue), allocator);
        }
        if(resultData.Place > 0)
        {
            doc.AddMember("place", resultData.Place, allocator);
        }
        rapidjson::Value steps_container(rapidjson::kArrayType);
        for( auto const & step : resultData.Steps)
        {
            rapidjson::Value step_object(rapidjson::kObjectType);
            step_object.AddMember("id", step.processId, allocator);
            step_object.AddMember("player", rapidjson::Value().SetBool(step.player), allocator);
            step_object.AddMember("step", step.step, allocator);
            step_object.AddMember("bulls", step.bulls, allocator);
            step_object.AddMember("cows", step.cows, allocator);
            step_object.AddMember("game_value", TStandartRules::Instance().gameValueToUint(step.gameValueList), allocator);
            step_object.AddMember("finished", rapidjson::Value().SetBool(step.finished), allocator);
            steps_container.PushBack(step_object, allocator);
        }
        doc.AddMember("steps", steps_container, allocator);
        if(resultData.GameIds.size() > 0)
        {
            rapidjson::Value games_container(rapidjson::kArrayType);
            for (auto id : resultData.GameIds)
            {
                games_container.PushBack(rapidjson::Value().SetUint(id), allocator);
            }
            doc.AddMember("game_ids", games_container, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    std::string getEnvVar(const std::string& name, const std::string& defaultValue = "")
    {
        const char* value = std::getenv(name.c_str());
        return value ? value : defaultValue;
    }
}
