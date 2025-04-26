#pragma once

#include <iostream>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>

#include "ui_game_view.h"
#include <memory>
#include <string>
#include <game_controller.h>

namespace MODEL_COMPONENTS
{
    enum class TGameMode : int32_t
    {
        UNKNOWN = -1,
        BEGIN = 0,
        PLAYER = BEGIN,
        COMPUTER,
        PLAYER_VS_COMPUTER,
        END,
    };
    std::ostream& operator<<( std::ostream& stream_, TGameMode _gameMode );
}

class TGameView : public QWidget
{
    Q_OBJECT

public:
    explicit TGameView( QWidget* _parent = nullptr );
    ~TGameView( ) override;

    void OnInitialUpdate( );

public slots :
    //ui
    void onSwitchGameMode(int _currentIndex);
    void onSwitchGameBrain(int _currentIndex);
    void onStartGame();
    void onSendGameValue();
    void onFinishGame();
    void onMakeStep();

private : //methods
    void writeResults( uint32_t _processId, std::vector< uint8_t> const & _gameValueList, uint32_t bulls, uint32_t cows, uint32_t _attemptNumber, bool _finished );
    void finishGameStep();
    void winMessageSend();
    void gameFinished();
    std::vector<QString> generateGameModesList() const;
    std::vector<QString> generateGameBrainsList() const;

    MODEL_COMPONENTS::TGameMode GameMode() const
    {
        return m_game_mode;
    }
    MODEL_COMPONENTS::TGameBrain GameBrain() const
    {
        return m_game_brain;
    }

//attributes
    Ui::TFormMainView ui{};
    std::shared_ptr<TGameController> m_game_controller{};
    MODEL_COMPONENTS::TGameMode m_game_mode = MODEL_COMPONENTS::TGameMode::UNKNOWN;
    MODEL_COMPONENTS::TGameBrain m_game_brain = MODEL_COMPONENTS::TGameBrain::NONE;
    std::vector<uint32_t> m_winIds{};
    uint32_t m_player_game_id = 0;
    uint32_t m_computer_game_id = 0;
};
