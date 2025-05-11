#pragma once
#include <components/enums.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include <common_operations.hpp>
#include <rules/standart_rules.hpp>
#include <cstdlib>

namespace SERVER_COMPONENTS
{
    TRequestData ParseKafkaMessage( std::string message );
    std::string SerializeResultToKafkaMessage(TResultData const & resultData);
    std::string getEnvVar(const std::string& name, const std::string& defaultValue);
}
