#pragma once
#include <cinttypes>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <cassert>
#include <core/enums.hpp>

namespace SERVER_COMPONENTS
{
    enum class TServerState : uint32_t
    {
        UNKNOWN = 0,
        WAIT_REGISTRATION,
        READY_TO_WORK,
    };
    std::ostream& operator<<( std::ostream& stream_, TServerState id );
    enum class TCommand : uint32_t
    {
        UNKNOWN = 0,
        SERVER_COMMAND_BEGIN,
        REGISTER_SERVER = SERVER_COMMAND_BEGIN,
        SERVER_INFO,
        DISCONNECT_SERVER,
        SERVER_COMMAND_END,
        CREATE_GAME = SERVER_COMMAND_END,
        ADD_PLAYER,
        ADD_COMPUTER,
        START_GAME,
        PLAYER_STEP,
        COMPUTER_STEP,
        SWITCH_BRAIN,
        PLAYER_GIVE_UP,
        STEP_INFO,
        GIVE_RIGHTS,
        LEAVE_FROM_GAME,
        FINISH_GAME,
        GAME_RESULT,
        REMOVE_GAME,
        RESTORE_GAME,
    };
    std::ostream& operator<<( std::ostream& stream_, TCommand id );
    enum class TResult : uint32_t
    {
        UNKNOWN = 0,
        COMPLETE,
        ERROR_PARSE_MESSAGE,
        ERROR_SERVER_NOT_REGISTRED,
        ERROR_SERVER_ALREADY_REGISTRED,
        ERROR_BAD_SERVER_ID,
        ERROR_MESSAGE_FOR_ANOTHER_SERVER,
        ERROR_UNKNOWN_COMMAND,
        ERROR_GAME_ALREADY_EXISTS,
        ERROR_PLAYER_NOT_FOUND,
        ERROR_GAME_NOT_EXISTS,
        ERROR_COMPUTER_NOT_FOUND,
        ERROR_INVALID_SECRET_VALUE,
        ERROR_INVALID_GAME_VALUE,
        ERROR_GAME_ALREADY_FINISHED,
        ERROR_PLAYER_ALREADY_FINISHED,
        ERROR_PLAYER_ALREADY_EXISTS,
        ERROR_COMPUTER_ALREADY_EXISTS,
        ERROR_NOT_ALL_PLAYERS_STEPED,
        ERROR_GAME_IN_PROGRESS,
        ERROR_PLAYER_ALREADY_MADE_STEP,
        ERROR_PLAYER_HAVE_UNDELETED_COMPUTERS,
        ERROR_PLAYER_DOESNT_HAVE_RIGHTS,
        ERROR_NOT_ALL_PLAYERS_LEAVE_FROM_GAME,
        ERROR_BRAIN_NOT_CHANGING_IN_PROGRESS,
        ERROR_INVALID_GAME_STEP,
        ERROR_GAMES_LIMIT_REACHED,
        ERROR_PROCESS_LIMIT_REACHED,
        ERROR_UNKNOWN_BRAIN,
    };
    std::ostream& operator<<( std::ostream& stream_, TResult id );


    struct TResultData
    {
        uint32_t ServerId = 0;
        std::string CorrelationId{};
        TResult Result = TResult::UNKNOWN;
        MODEL_COMPONENTS::TGameStage GameStage = MODEL_COMPONENTS::TGameStage::UNKNOWN;
        std::vector< MODEL_COMPONENTS::StepResults > Steps;
        std::vector< MODEL_COMPONENTS::GameResults > GameResults;
        uint32_t PlayerId = 0;
        uint32_t GameId = 0;
        uint32_t Players = 0;
        uint32_t UnsteppedPlayers = 0;
        std::vector<uint8_t> SecretValue{};
        std::vector<uint32_t> GameIds{};
    };

    std::ostream& operator<<( std::ostream& stream_, TResultData const & result );
    struct TRequestData
    {
        TCommand Command = TCommand::UNKNOWN;
        std::string CorrelationId{};
        uint32_t ServerId = 0;
        uint32_t PlayerId = 0;
        uint32_t GameId = 0;
        uint32_t ComputerId = 0;
        std::map< uint32_t, bool > Players;
        std::vector< MODEL_COMPONENTS::StepResults > History;
        std::map< uint32_t , std::pair< uint32_t, MODEL_COMPONENTS::TGameBrain> > BrainsMap;
        uint32_t Step = 0;
        MODEL_COMPONENTS::TGameBrain GameBrain = MODEL_COMPONENTS::TGameBrain::NONE;
        std::vector<uint8_t> GameValue{};
    };
    std::ostream& operator<<( std::ostream& stream_, TRequestData const & request );
}
