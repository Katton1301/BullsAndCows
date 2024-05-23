#include "game_view.h"
#include "common_operations.hpp"
#include <QtWidgets/QMessageBox>

QTableWidgetItem * createTableItem(QString text)
{
    auto item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

inline std::string GameBrainToLevelName( MODEL_COMPONENTS::TGameBrain _gameMode )
{
    switch ( _gameMode ) {
        case MODEL_COMPONENTS::TGameBrain::RANDOM :
            return std::string("Easiest");
            break;
        case MODEL_COMPONENTS::TGameBrain::STUPID :
            return std::string("Easy");
            break;
        case MODEL_COMPONENTS::TGameBrain::SMART :
            return std::string("Medium");
            break;
        case MODEL_COMPONENTS::TGameBrain::BEST :
            return std::string("Hard");
            break;
        default :
            return std::string("UNKNOWN");
            break;
    }
}

TGameView::TGameView( QWidget* _parent )
    : QWidget( _parent )
    , m_game_controller(new TGameController)
{
    ui.setupUi( this );
    ui.historyComputerTable->setColumnCount(4);
    ui.historyComputerTable->setHorizontalHeaderItem(0, createTableItem("i"));
    ui.historyComputerTable->setHorizontalHeaderItem(1, createTableItem("Bulls"));
    ui.historyComputerTable->setHorizontalHeaderItem(2, createTableItem("Cows"));
    ui.historyComputerTable->setHorizontalHeaderItem(3, createTableItem("Number"));
    ui.historyComputerTable->clearContents();
    ui.historyComputerTable->setRowCount(0);
    ui.historyComputerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui.historyPlayerTable->setColumnCount(4);
    ui.historyPlayerTable->setHorizontalHeaderItem(0, createTableItem("i"));
    ui.historyPlayerTable->setHorizontalHeaderItem(1, createTableItem("Bulls"));
    ui.historyPlayerTable->setHorizontalHeaderItem(2, createTableItem("Cows"));
    ui.historyPlayerTable->setHorizontalHeaderItem(3, createTableItem("Number"));
    ui.historyPlayerTable->clearContents();
    ui.historyPlayerTable->setRowCount(0);
    ui.historyPlayerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


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
        SIGNAL( sendAttemptResults(MODEL_COMPONENTS::TGameMode, std::vector< uint8_t> const &, uint32_t, uint32_t, uint32_t) ),
        this,
        SLOT( onAttemptResults(MODEL_COMPONENTS::TGameMode, std::vector< uint8_t> const &, uint32_t, uint32_t, uint32_t) )
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
    ui.computerHistoryFrame->setVisible(false);
    ui.playerHistoryFrame->setVisible(false);
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

    ui.historyPlayerTable->clearContents();
    ui.historyPlayerTable->setRowCount(0);
    ui.historyComputerTable->clearContents();
    ui.historyComputerTable->setRowCount(0);

    ui.computerHistoryFrame->setVisible(m_game_controller->GameMode() != MODEL_COMPONENTS::TGameMode::PLAYER);

    ui.playerHistoryFrame->setVisible(m_game_controller->GameMode() != MODEL_COMPONENTS::TGameMode::COMPUTER);

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
    std::string msgText = "Game was finished.";
    switch(_gameMode)
    {
    case MODEL_COMPONENTS::TGameMode::COMPUTER:
        msgText = "Game was finished. Computer found correct number";
        break;
    case MODEL_COMPONENTS::TGameMode::PLAYER:
        msgText = "Game was finished. Player found correct number";
        break;
    case MODEL_COMPONENTS::TGameMode::PLAYER_VS_COMPUTER:
        msgText = "Game was finished. Draw! Both found correct number";
        break;
    default:
        break;
    }

    messageOk.setText( QString( msgText.c_str()) );
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

    MODEL_COMPONENTS::TGameBrain gameBrain = MODEL_COMPONENTS::TGameBrain::BEGIN;
    for (
        uint32_t id  = static_cast< uint32_t >( MODEL_COMPONENTS::TGameBrain::BEGIN );
        id != static_cast< uint32_t >( MODEL_COMPONENTS::TGameBrain::END );
        ++id
    )
    {
        if ( GameBrainToLevelName(static_cast< MODEL_COMPONENTS::TGameBrain >( id )) == ui.gameBrainComboBox->currentText().toStdString() )
        {
            gameBrain = static_cast< MODEL_COMPONENTS::TGameBrain >( id );
            break;
        }
    }
    emit switchGameBrain(gameBrain);
}

void TGameView::onAttemptResults(MODEL_COMPONENTS::TGameMode _gameMode, std::vector< uint8_t> const & _gameValueList, uint32_t bulls, uint32_t cows, uint32_t _attemptNumber )
{
    assert(
            _gameMode == MODEL_COMPONENTS::TGameMode::COMPUTER ||
            _gameMode == MODEL_COMPONENTS::TGameMode::PLAYER
        );

    auto * table = _gameMode == MODEL_COMPONENTS::TGameMode::COMPUTER
                ? ui.historyComputerTable
                : ui.historyPlayerTable
                ;

    int newRow = table->rowCount();
    if(_gameMode == MODEL_COMPONENTS::TGameMode::COMPUTER && _attemptNumber > 10)
    {
        if(_attemptNumber == 11)
        {
            table->item(8, 0)->setText(QString("..."));
            table->item(8, 1)->setText(QString("..."));
            table->item(8, 2)->setText(QString("..."));
            table->item(8, 3)->setText(QString("..."));
        }
        --newRow;
    }


    table->setRowCount(newRow + 1);
    table->setItem(newRow, 0, createTableItem(QString("%1").arg(_attemptNumber)));
    table->setItem(newRow, 1, createTableItem(QString("%1").arg(bulls)));
    table->setItem(newRow, 2, createTableItem(QString("%1").arg(cows)));
    if(_gameMode == MODEL_COMPONENTS::TGameMode::COMPUTER)
    {
        table->setItem(newRow, 3, createTableItem(QString("* * * *")));
    }
    else
    {
        table->setItem(newRow, 3, createTableItem(QString("%1").arg(QString("%1 %2 %3 %4")
            .arg(_gameValueList.at(0))
            .arg(_gameValueList.at(1))
            .arg(_gameValueList.at(2))
            .arg(_gameValueList.at(3))
        )));
    }
}

void TGameView::onSendGameValue()
{
    std::vector< uint8_t > gameValueList;
    gameValueList.push_back(ui.digit_1->text().toUInt());
    gameValueList.push_back(ui.digit_2->text().toUInt());
    gameValueList.push_back(ui.digit_3->text().toUInt());
    gameValueList.push_back(ui.digit_4->text().toUInt());

    m_game_controller->PlayerStep(gameValueList);
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
        auto gameBrainStr = GameBrainToLevelName(static_cast<MODEL_COMPONENTS::TGameBrain>(gameBrainId));
        gameBrainsList.push_back(QString(gameBrainStr.c_str()));
    }
    return gameBrainsList;
}
