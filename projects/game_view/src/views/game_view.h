#pragma once

#include <iostream>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>

#include "ui_game_view.h"
#include <memory>
#include <string>
#include "controllers/game_controller.h"

class TGameView : public QWidget
{
    Q_OBJECT

public:
    explicit TGameView( QWidget* _parent = nullptr );
    ~TGameView( ) override;

    void OnInitialUpdate( );

signals:
    void switchGameMode(MODEL_COMPONENTS::TGameMode _gameMode);
    void switchGameBrain(MODEL_COMPONENTS::TGameBrain _gameBrain);

public slots :
    void onSwitchGameMode(int _currentIndex);
    void onSwitchGameBrain(int _currentIndex);
    void onStartGame();
    void onSendGameValue();
    void onMakeStep();
    void onAttemptResults(MODEL_COMPONENTS::TGameMode _gameMode, std::vector< uint8_t> const & _gameValueList, uint32_t bulls, uint32_t cows, uint32_t _attemptNumber );
    void onGameStarted(MODEL_COMPONENTS::TGameMode _gameMode);
    void onGameFinished(MODEL_COMPONENTS::TGameMode _gameMode);

private :
//methods
    std::vector<QString> generateGameModesList() const;
    std::vector<QString> generateGameBrainsList() const;

//attributes
    Ui::TFormMainView ui{};
    std::unique_ptr<TGameController> m_game_controller{};

};
