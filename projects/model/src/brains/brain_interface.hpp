#pragma once
#include <cstdint>
struct IGameBrain
{
    virtual void Init() = 0;
    virtual void makePredict( ) = 0;
};
