#include "game_brain.hpp"
#include "game_process.hpp"
#include <algorithm>

TStandartRandomBrain::TStandartRandomBrain( TStandartGameProcess * _gameProcess )
    : m_gameProcess_ptr(_gameProcess)
    , m_possibleValues()
{
}

void TStandartRandomBrain::Init( )
{
    copyPossibleValuesList();
}

void TStandartRandomBrain::makePredict()
{
    assert(m_possibleValues.size() > 0);
    auto rnd_offset = m_gameProcess_ptr->RandomByModulus()(m_possibleValues.size());
    m_gameProcess_ptr->appendGameValue( m_possibleValues.at(rnd_offset) );
    m_possibleValues.erase( m_possibleValues.begin() + rnd_offset );
}

void TStandartRandomBrain::copyPossibleValuesList( )
{

    m_possibleValues.clear();
    for(auto const & gameValue : TStandartRules::Instance()->AllPossibleGameValues())
    {
        m_possibleValues.push_back(gameValue);
    }
}

std::shared_ptr<IGameBrain> createBrain( TStandartGameProcess * _gameProcess, MODEL_COMPONENTS::TGameBrain _gameBrain )
{
    std::shared_ptr<IGameBrain> gameBrain = nullptr;
    switch (_gameBrain) {
    case MODEL_COMPONENTS::TGameBrain::RANDOM:
        gameBrain = std::make_shared<TStandartRandomBrain>(_gameProcess);
        break;
    default:
        gameBrain = std::make_shared<TStandartRandomBrain>(_gameProcess);
        break;
    }
    return gameBrain;
}
