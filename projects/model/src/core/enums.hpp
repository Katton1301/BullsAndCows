#pragma once
#include <iostream>
#include <ctime>

namespace MODEL_COMPONENTS
{
    enum class TGameStage : int32_t
    {
        UNKNOWN = -1,
        WAIT_A_NUMBER,
        IN_PROGRESS,
        FINISHED,
    };
    std::ostream& operator<<( std::ostream& stream_, TGameStage _gameStage );

    enum class TGameBrain : int32_t
    {
        UNKNOWN = -1,
        BEGIN,
        RANDOM = BEGIN,
        STUPID,
        SMART,
        END,
    };
    std::ostream& operator<<( std::ostream& stream_, TGameBrain _gameBrain );
}
