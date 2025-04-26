#include <views/game_view.h>
#include <common_operations.hpp>
#include <QtWidgets/QMessageBox>

QTableWidgetItem * createTableItem(QString text)
{
    auto item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

namespace MODEL_COMPONENTS
{
    std::ostream & operator<< ( std::ostream & stream_, TGameMode _gameMode )
    {
        switch( _gameMode )
        {
        case TGameMode::UNKNOWN    :
            stream_ << "UNKNOWN";
            break;

        case TGameMode::PLAYER  :
            stream_ << "PLAYER";
            break;

        case TGameMode::COMPUTER  :
            stream_ << "COMPUTER";
            break;

        case TGameMode::PLAYER_VS_COMPUTER  :
            stream_ << "PLAYER_VS_COMPUTER";
            break;

        default :
            stream_ << "[Error] identifier of game mode is wrong : " << static_cast< int32_t >( _gameMode ) << std::endl;
            assert( false && " incorrect game mode identifier" );
            break;
        }
        return stream_;
    }
}

TGameView::TGameView( QWidget* _parent )
    : QWidget( _parent )
{
    m_game_controller = std::make_shared<TGameController>();
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

    m_game_controller->InitGame();
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
    m_game_brain = MODEL_COMPONENTS::TGameBrain::BEGIN;
    for(auto const & gameBrain : generateGameBrainsList())
    {
        ui.gameBrainComboBox->addItem(gameBrain);
    }
    ui.computerHistoryFrame->setVisible(false);
    ui.playerHistoryFrame->setVisible(false);
    ui.digitsFrame->setVisible(false);
    ui.sendButton->setVisible(false);
    ui.finishButton->setVisible(false);
    ui.makeStepButton->setVisible(false);
    ui.startButton->setEnabled(true);
    ui.gameModeComboBox->setEnabled(true);
}

void TGameView::onStartGame()
{
    m_game_controller->InitGame();

    ui.historyPlayerTable->clearContents();
    ui.historyPlayerTable->setRowCount(0);
    ui.historyComputerTable->clearContents();
    ui.historyComputerTable->setRowCount(0);

    ui.computerHistoryFrame->setVisible(GameMode() != MODEL_COMPONENTS::TGameMode::PLAYER);

    ui.playerHistoryFrame->setVisible(GameMode() != MODEL_COMPONENTS::TGameMode::COMPUTER);
    m_winIds.clear();

    auto error = m_game_controller->StartGame();
    assert(error == TGameController::TError::OK);
    ui.startButton->setEnabled(false);
    ui.gameModeComboBox->setEnabled(false);
    ui.gameBrainComboBox->setEnabled(false);
    ui.finishButton->setVisible(false);

    if(GameMode() == MODEL_COMPONENTS::TGameMode::COMPUTER)
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

void TGameView::gameFinished()
{
    ui.digitsFrame->setVisible(false);
    ui.sendButton->setVisible(false);
    ui.finishButton->setVisible(false);
    ui.makeStepButton->setVisible(false);
    ui.startButton->setEnabled(true);
    ui.gameModeComboBox->setEnabled(true);
    ui.gameBrainComboBox->setEnabled(GameMode() != MODEL_COMPONENTS::TGameMode::PLAYER);
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

    m_game_controller->InitGame();
    m_game_mode = gameMode;
    if(GameMode() != MODEL_COMPONENTS::TGameMode::PLAYER && m_computer_game_id == 0)
    {
        m_computer_game_id = 2;
        auto error = m_game_controller->addComputerProcess(m_computer_game_id, GameBrain());
        assert(error == TGameController::TError::OK);
    }

    if(GameMode() != MODEL_COMPONENTS::TGameMode::COMPUTER && m_player_game_id == 0)
    {
        m_player_game_id = 1;
        auto error = m_game_controller->addPlayerProcess(m_player_game_id);
        assert(error == TGameController::TError::OK);
    }

    if(GameMode() == MODEL_COMPONENTS::TGameMode::PLAYER && m_computer_game_id > 0)
    {
        auto error = m_game_controller->removePlayerProcess(m_computer_game_id, false);
        assert(error == TGameController::TError::OK);
        m_computer_game_id = 0;
    }

    if(GameMode() == MODEL_COMPONENTS::TGameMode::COMPUTER && m_player_game_id > 0)
    {
        auto error = m_game_controller->removePlayerProcess(m_player_game_id, true);
        assert(error == TGameController::TError::OK);
        m_player_game_id = 0;
    }
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
        if ( COMMON_OPERATIONS::GameBrainToLevelName(static_cast< MODEL_COMPONENTS::TGameBrain >( id )) == ui.gameBrainComboBox->currentText().toStdString() )
        {
            gameBrain = static_cast< MODEL_COMPONENTS::TGameBrain >( id );
            break;
        }
    }
    m_game_controller->InitGame();
    if(GameMode() != MODEL_COMPONENTS::TGameMode::PLAYER)
    {
        if( m_computer_game_id == 0)
        {
            m_computer_game_id = 2;
            auto error = m_game_controller->addComputerProcess(m_computer_game_id, gameBrain);
            assert(error == TGameController::TError::OK);
        }
        else
        {
            auto error = m_game_controller->switchGameBrain( m_computer_game_id, gameBrain);
            assert(error == TGameController::TError::OK);
        }
    }
}

void TGameView::writeResults( uint32_t _processId, std::vector< uint8_t> const & _gameValueList, uint32_t bulls, uint32_t cows, uint32_t _attemptNumber, [[maybe_unused]]bool _finished  )
{
    QTableWidget * table = nullptr;

    if(_processId == m_player_game_id)
    {
        table = ui.historyPlayerTable;
    }
    if(_processId == m_computer_game_id)
    {
        table = ui.historyComputerTable;
    }
    assert(table != nullptr);

    int newRow = table->rowCount();
    if(_processId == m_computer_game_id && _attemptNumber > 10)
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
    if(GameMode() == MODEL_COMPONENTS::TGameMode::PLAYER_VS_COMPUTER && _processId == m_computer_game_id)
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

void TGameView::finishGameStep()
{
    for(auto const & result : m_game_controller->getStepResults(m_game_controller->GameStep()))
    {
        writeResults(result.processId, result.gameValueList, result.bulls, result.cows, result.step, result.finished);
        if(result.finished)
        {
            QTableWidget * table = nullptr;

            if(result.processId == m_player_game_id)
            {
                table = ui.historyPlayerTable;
            }
            if(result.processId == m_computer_game_id)
            {
                table = ui.historyComputerTable;
            }
            assert(table != nullptr);
            table->item(result.step - 1, 3)->setBackground(Qt::green);
        }

    }
    if(m_winIds.size() == 0)
    {
        for( auto [winId, isPlayer] : m_game_controller->WinnersId() )
        {
            m_winIds.push_back(winId);
        }

        if(m_winIds.size() > 0)
        {
            winMessageSend();
            ui.finishButton->setVisible(true);
        }
    }
}

void TGameView::winMessageSend()
{
    uint32_t winMask = 0;
    if(std::ranges::any_of(m_winIds, [this](auto id){ return id == m_player_game_id;}))
    {
        winMask += 1;
    }
    if(std::ranges::any_of(m_winIds, [this](auto id){ return id == m_computer_game_id;}))
    {
        winMask += 2;
    }

    QMessageBox messageOk;
    std::string msgText = "Game was finished.";
    switch(winMask)
    {
    case 2:
        msgText = "Game was finished. Computer found correct number";
        break;
    case 1:
        msgText = "Game was finished. Player found correct number";
        break;
    case 3:
        msgText = "Game was finished. Draw! Both found correct number";
        break;
    default:
        break;
    }

    messageOk.setText( QString( msgText.c_str()) );
    messageOk.exec();
}

void TGameView::onSendGameValue()
{
    std::vector< uint8_t > gameValueList;
    gameValueList.push_back(ui.digit_1->text().toUInt());
    gameValueList.push_back(ui.digit_2->text().toUInt());
    gameValueList.push_back(ui.digit_3->text().toUInt());
    gameValueList.push_back(ui.digit_4->text().toUInt());

    auto error = m_game_controller->DoPlayerStep(m_player_game_id, gameValueList);
    assert(error == TGameController::TError::OK);
    finishGameStep();
    if(
        m_game_controller->GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS_WINNER_DEFINED &&
        std::ranges::any_of(m_winIds, [this](auto id){ return id == m_player_game_id;})
        )
    {
        while(m_game_controller->GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED)
        {
            error = m_game_controller->FinishStep();
            assert(error == TGameController::TError::OK);
            finishGameStep();
        }
    }
    if(m_game_controller->GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED)
    {
        gameFinished();
    }
}

void TGameView::onFinishGame()
{
    m_game_controller->FinishGame();
    gameFinished();
}

void TGameView::onMakeStep()
{
    auto error = m_game_controller->FinishStep();
    assert(error == TGameController::TError::OK);
    finishGameStep();
    if(m_game_controller->GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED)
    {
        gameFinished();
    }
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
        auto gameBrainStr = COMMON_OPERATIONS::GameBrainToLevelName(static_cast<MODEL_COMPONENTS::TGameBrain>(gameBrainId));
        gameBrainsList.push_back(QString(gameBrainStr.c_str()));
    }
    return gameBrainsList;
}
