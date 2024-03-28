#pragma once
#include "enums.hpp"
#include "elements/game_process.hpp"
#include <QObject>

namespace MODEL_COMPONENTS
{
    enum class TGameMode : int32_t
    {
        UNKNOWN = -1,
        BEGIN = 0,
        PLAYER = BEGIN,
        COMPUTER,
        END,
    };
    std::ostream& operator<<( std::ostream& stream_, TGameMode _gameMode );
}

class TGameController : public QObject
{
    Q_OBJECT
public:
    TGameController();
    ~TGameController();

    void InitGame();
    void CreateGame();
    void appendGameValue( std::vector< uint8_t > const & _gameValueList );
    void makeStep();
    MODEL_COMPONENTS::TGameStage GameStage() const;
    MODEL_COMPONENTS::TGameMode GameMode() const;

signals:
    void sendAttemptResults( std::vector< uint8_t> const & _gameValueList, uint32_t bulls, uint32_t cows, uint32_t _attemptNumber );
    void gameStarted(MODEL_COMPONENTS::TGameMode _gameMode);
    void gameFinished(MODEL_COMPONENTS::TGameMode _gameMode);

public slots:
    void gameModeSwitched(MODEL_COMPONENTS::TGameMode _gameMode);
    void gameBrainSwitched(MODEL_COMPONENTS::TGameBrain _gameBrain);

private:
    void handleResults();
    TStandartGameProcess & GameProcess_ref( )
    {
        return *m_game_process;
    }
    TStandartGameProcess & GameProcess_cref( ) const
    {
        return *m_game_process;
    }

    void setGameMode( MODEL_COMPONENTS::TGameMode _gameMode );
    MODEL_COMPONENTS::TGameBrain GameBrain() const;
    void setGameBrain( MODEL_COMPONENTS::TGameBrain _gameBrain );

private:
    std::shared_ptr<TStandartGameProcess> m_game_process{};
    MODEL_COMPONENTS::TGameMode m_gameMode = MODEL_COMPONENTS::TGameMode::UNKNOWN;
    MODEL_COMPONENTS::TGameBrain m_gameBrain = MODEL_COMPONENTS::TGameBrain::UNKNOWN;
};
