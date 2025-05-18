#pragma once
#include <cstdint>
#include <core/game_value.hpp>
struct IGameBrain
{
    virtual void Init() = 0;
    virtual void makePredict( ) = 0;
    virtual void restoreFromHistory() = 0;
};
