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

        if (doc.HasMember("restore_data") && doc["restore_data"].IsArray())
        {
            for (auto const & item : doc["restore_data"].GetArray())
            {
                MODEL_COMPONENTS::StepResults step_result;
                if(item.HasMember("game_value"))
                {
                    rapidjson::Value const & gameValueJson = item["game_value"];
                    std::vector<uint8_t> gameValue;
                    if(gameValueJson.IsUint())
                    {
                        uint32_t gameValueInt = gameValueJson.GetUint();
                        gameValue.resize(TStandartRules::Instance().ValueSize());
                        for(int32_t i = TStandartRules::Instance().ValueSize() - 1; i >= 0; --i)
                        {
                            gameValue[i] = gameValueInt % 10;
                            gameValueInt /= 10;
                        }
                    }
                    step_result.gameValueList = gameValue;
                }
                if(item.HasMember("id"))
                {
                    step_result.processId = item["id"].GetUint();
                }
                if(item.HasMember("player"))
                {
                    step_result.player = item["player"].GetBool();
                }
                if(item.HasMember("bulls"))
                {
                    step_result.bulls = item["bulls"].GetUint();
                }
                if(item.HasMember("cows"))
                {
                    step_result.cows = item["cows"].GetUint();
                }
                if(item.HasMember("step"))
                {
                    step_result.step = item["step"].GetUint();
                }
                if(item.HasMember("finished"))
                {
                    step_result.finished = item["finished"].GetBool();
                }
                request.History.push_back(step_result);
            }
        }

        if(doc.HasMember("brains") && doc["brains"].IsArray())
        {
            for (auto const & item : doc["brains"].GetArray())
            {
                if(item.HasMember("id") && item.HasMember("brain") && item.HasMember("player_id"))
                {
                    request.BrainsMap.emplace(
                        item["id"].GetUint(),
                        std::make_pair(
                            item["player_id"].GetUint(),
                            COMMON_OPERATIONS::LevelNameToGameBrain(item["brain"].GetString())
                        )
                    );
                }
            }
        }

        if (doc.HasMember("game_value"))
        {
            rapidjson::Value const & gameValueJson = doc["game_value"];
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
        if(resultData.UnsteppedPlayers > 0)
        {
            doc.AddMember("unstepped_players", resultData.UnsteppedPlayers, allocator);
        }
        if(resultData.SecretValue.size() > 0)
        {
            doc.AddMember("secret_value", TStandartRules::Instance().gameValueToUint(resultData.SecretValue), allocator);
        }
        if(resultData.Steps.size() > 0)
        {
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
        }
        if(resultData.GameResults.size() > 0)
        {
            rapidjson::Value results_container(rapidjson::kArrayType);
            for( auto const & step : resultData.GameResults)
            {
                rapidjson::Value result_object(rapidjson::kObjectType);
                result_object.AddMember("id", step.processId, allocator);
                result_object.AddMember("player", rapidjson::Value().SetBool(step.player), allocator);
                result_object.AddMember("step", step.step, allocator);
                result_object.AddMember("place", step.place, allocator);
                result_object.AddMember("give_up", rapidjson::Value().SetBool(step.give_up), allocator);
                results_container.PushBack(result_object, allocator);
            }
            doc.AddMember("game_results", results_container, allocator);
        }
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
