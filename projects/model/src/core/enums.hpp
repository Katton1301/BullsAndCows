#pragma once
#include <iostream>
#include <ctime>
#include <cinttypes>
#include <vector>

namespace MODEL_COMPONENTS
{
    enum class TPlayerState : int32_t
    {
        UNKNOWN = -1,
        WAIT_A_NUMBER,
        IN_PROGRESS,
        GAVE_UP,
        FINISHED,
    };
    std::ostream& operator<<( std::ostream& stream_, TPlayerState _playerState );

    enum class TGameStage : int32_t
    {
        UNKNOWN = -1,
        WAIT_A_NUMBER,
        IN_PROGRESS,
        IN_PROGRESS_WINNER_DEFINED,
        FINISHED,
    };
    std::ostream& operator<<( std::ostream& stream_, TGameStage _gameStage );
    std::string gameStageToString(TGameStage _gameStage);

    enum class TGameBrain : int32_t
    {
        NONE = -1,
        BEGIN,
        RANDOM = BEGIN,
        STUPID,
        SMART,
        BEST,
        END,
    };
    std::ostream& operator<<( std::ostream& stream_, TGameBrain _gameBrain );

    struct StepResults
    {
        uint32_t processId = 0;
        bool player = true;
        uint32_t step = 0;
        std::vector< uint8_t> gameValueList{};
        uint32_t bulls = 0;
        uint32_t cows = 0;
        bool finished = false;
    };

    struct GameResults
    {
        uint32_t processId = 0;
        bool player = true;
        uint32_t step = 0;
        bool give_up = false;
        uint32_t place = 0;
    };
}
