#include "game_view.h"
#include "common_operations.hpp"
#include <QtWidgets/QMessageBox>

QTableWidgetItem * createTableItem(QString text)
{
    auto item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignHCenter);
    return item;
}

TGameView::TGameView( QWidget* _parent )
    : QWidget( _parent )
    , m_game_controller(new TGameController)
{
    ui.setupUi( this );
    ui.historyTable->setColumnCount(4);
    ui.historyTable->setHorizontalHeaderItem(0, createTableItem("i"));
    ui.historyTable->setHorizontalHeaderItem(1, createTableItem("Bulls"));
    ui.historyTable->setHorizontalHeaderItem(2, createTableItem("Cows"));
    ui.historyTable->setHorizontalHeaderItem(3, createTableItem("Number"));
    ui.historyTable->clearContents();
    ui.historyTable->setRowCount(0);
    ui.historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    QObject::connect(
        this,
        SIGNAL( switchGameMode(MODEL_COMPONENTS::TGameMode) ),
        &(*m_game_controller),
        SLOT( gameModeSwitched(MODEL_COMPONENTS::TGameMode) )
    );
    QObject::connect(
        this,
        SIGNAL( switchGameBrain(MODEL_COMPONENTS::TGameBrain) ),
        &(*m_game_controller),
        SLOT( gameBrainSwitched(MODEL_COMPONENTS::TGameBrain) )
    );

    QObject::connect(
        &(*m_game_controller),
        SIGNAL( gameStarted(MODEL_COMPONENTS::TGameMode) ),
        this,
        SLOT( onGameStarted(MODEL_COMPONENTS::TGameMode) )
    );

    QObject::connect(
        &(*m_game_controller),
        SIGNAL( gameFinished(MODEL_COMPONENTS::TGameMode) ),
        this,
        SLOT( onGameFinished(MODEL_COMPONENTS::TGameMode) )
    );

    QObject::connect(
        &(*m_game_controller),
        SIGNAL( sendAttemptResults(std::vector< uint8_t> const &, uint32_t, uint32_t, uint32_t) ),
        this,
        SLOT( onAttemptResults(std::vector< uint8_t> const &, uint32_t, uint32_t, uint32_t) )
    );

}

TGameView::~TGameView( )
{
}

void TGameView::OnInitialUpdate( )
{
    for(auto const & gameMode : generateGameModesList())
    {
        ui.gameModeComboBox->addItem(gameMode);
    }
    for(auto const & gameBrain : generateGameBrainsList())
    {
        ui.gameBrainComboBox->addItem(gameBrain);
    }

    ui.digitsFrame->setVisible(false);
    ui.sendButton->setVisible(false);
    ui.makeStepButton->setVisible(false);
    ui.startButton->setEnabled(true);
    ui.gameModeComboBox->setEnabled(true);
}

void TGameView::onStartGame()
{
    if(m_game_controller->GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED)
    {
        m_game_controller->InitGame();
    }
    ui.historyTable->clearContents();
    ui.historyTable->setRowCount(0);
    m_game_controller->CreateGame();
    ui.startButton->setEnabled(false);
    ui.gameModeComboBox->setEnabled(false);
    ui.gameBrainComboBox->setEnabled(false);
}

void TGameView::onGameStarted(MODEL_COMPONENTS::TGameMode _gameMode)
{
    if(_gameMode == MODEL_COMPONENTS::TGameMode::COMPUTER)
    {
        ui.digitsFrame->setVisible(false);
        ui.sendButton->setVisible(false);
        ui.makeStepButton->setVisible(true);
    }
    else
    {
        ui.digitsFrame->setVisible(true);
        ui.sendButton->setVisible(true);
        ui.makeStepButton->setVisible(false);
    }
}

void TGameView::onGameFinished(MODEL_COMPONENTS::TGameMode _gameMode)
{

    QMessageBox messageOk;
    std::stringstream streamGameMode;
    std::string strGameMode;
    streamGameMode << _gameMode;
    streamGameMode >> strGameMode;
    messageOk.setText( QString( "Game was finished. %1 find correct number" ).arg(strGameMode.c_str()) );
    messageOk.exec();

    ui.digitsFrame->setVisible(false);
    ui.sendButton->setVisible(false);
    ui.makeStepButton->setVisible(false);
    ui.startButton->setEnabled(true);
    ui.gameModeComboBox->setEnabled(true);
    ui.gameBrainComboBox->setEnabled(m_game_controller->GameMode() != MODEL_COMPONENTS::TGameMode::PLAYER);
}

void TGameView::onSwitchGameMode([[maybe_unused]] int _currentIndex)
{
    MODEL_COMPONENTS::TGameMode gameMode = COMMON_OPERATIONS::convertFromStdString(
                    ui.gameModeComboBox->currentText().toStdString(),
                    MODEL_COMPONENTS::TGameMode::BEGIN,
                    MODEL_COMPONENTS::TGameMode::BEGIN,
                    MODEL_COMPONENTS::TGameMode::END
                );
    ui.gameBrainComboBox->setEnabled(gameMode != MODEL_COMPONENTS::TGameMode::PLAYER);
    emit switchGameMode(gameMode);
}

void TGameView::onSwitchGameBrain([[maybe_unused]] int _currentIndex)
{
    MODEL_COMPONENTS::TGameBrain gameBrain = COMMON_OPERATIONS::convertFromStdString(
                    ui.gameBrainComboBox->currentText().toStdString(),
                    MODEL_COMPONENTS::TGameBrain::BEGIN,
                    MODEL_COMPONENTS::TGameBrain::BEGIN,
                    MODEL_COMPONENTS::TGameBrain::END
                );
    emit switchGameBrain(gameBrain);
}

void TGameView::onAttemptResults( std::vector< uint8_t> const & _gameValueList, uint32_t bulls, uint32_t cows, uint32_t _attemptNumber )
{
    ui.historyTable->setRowCount(ui.historyTable->rowCount() + 1);
    ui.historyTable->setItem(ui.historyTable->rowCount() - 1, 0, createTableItem(QString("%1").arg(_attemptNumber)));
    ui.historyTable->setItem(ui.historyTable->rowCount() - 1, 1, createTableItem(QString("%1").arg(bulls)));
    ui.historyTable->setItem(ui.historyTable->rowCount() - 1, 2, createTableItem(QString("%1").arg(cows)));
    ui.historyTable->setItem(ui.historyTable->rowCount() - 1, 3, createTableItem(QString("%1").arg(QString("%1 %2 %3 %4")
        .arg(_gameValueList.at(0))
        .arg(_gameValueList.at(1))
        .arg(_gameValueList.at(2))
        .arg(_gameValueList.at(3))
    )));
    ui.numberVariations->setText(QString("%1").arg(_attemptNumber));
}

void TGameView::onSendGameValue()
{
    std::vector< uint8_t > gameValueList;
    gameValueList.push_back(ui.digit_1->text().toUInt());
    gameValueList.push_back(ui.digit_2->text().toUInt());
    gameValueList.push_back(ui.digit_3->text().toUInt());
    gameValueList.push_back(ui.digit_4->text().toUInt());

    m_game_controller->appendGameValue(gameValueList);
}

void TGameView::onMakeStep()
{
    m_game_controller->makeStep();
}

std::vector<QString> TGameView::generateGameModesList()  const
{
    std::vector<QString> gameModesList;
    for(
        uint32_t gameModeId = static_cast<uint32_t>(MODEL_COMPONENTS::TGameMode::BEGIN);
        gameModeId < static_cast<uint32_t>(MODEL_COMPONENTS::TGameMode::END);
        ++gameModeId
    )
    {
        auto gameModeStr = COMMON_OPERATIONS::convertToStdString(static_cast<MODEL_COMPONENTS::TGameMode>(gameModeId));
        gameModesList.push_back(QString(gameModeStr.c_str()));
    }
    return gameModesList;
}



std::vector<QString> TGameView::generateGameBrainsList()  const
{
    std::vector<QString> gameBrainsList;
    for(
        uint32_t gameBrainId = static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::BEGIN);
        gameBrainId < static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::END);
        ++gameBrainId
    )
    {
        auto gameBrainStr = COMMON_OPERATIONS::convertToStdString(static_cast<MODEL_COMPONENTS::TGameBrain>(gameBrainId));
        gameBrainsList.push_back(QString(gameBrainStr.c_str()));
    }
    return gameBrainsList;
}
