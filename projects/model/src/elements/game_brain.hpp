#pragma once
#include "game_brain_interface.hpp"
#include "game_value.hpp"
#include "enums.hpp"
#include <memory>

class TStandartGameProcess;

class TStandartRandomBrain : public IGameBrain
{
public:
    TStandartRandomBrain() = delete;
    TStandartRandomBrain( TStandartGameProcess * _gameProcess );
    ~TStandartRandomBrain() = default;

    void Init( ) override;

    void makePredict() override;

private:
    void copyPossibleValuesList();

private:
    TStandartGameProcess * m_gameProcess_ptr = nullptr;
    std::vector< TGameValue<uint8_t> > m_possibleValues{};
};

std::shared_ptr<IGameBrain> createBrain( TStandartGameProcess * _gameProcess, MODEL_COMPONENTS::TGameBrain _gameBrain );
