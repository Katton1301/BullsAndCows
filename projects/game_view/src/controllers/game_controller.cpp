#include "game_controller.h"

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


TGameController::TGameController()
    : m_player_game_process(std::make_shared<TStandartGameProcess>())
    , m_computer_game_process(std::make_shared<TStandartGameProcess>())
    , m_gameMode(MODEL_COMPONENTS::TGameMode::UNKNOWN)
    , m_gameBrain(MODEL_COMPONENTS::TGameBrain::UNKNOWN)
{
    InitGame();
}

TGameController::~TGameController()
{

}

MODEL_COMPONENTS::TGameMode TGameController::GameMode() const
{
    return m_gameMode;
}

void TGameController::setGameMode( MODEL_COMPONENTS::TGameMode _gameMode )
{
    m_gameMode = _gameMode;
}

MODEL_COMPONENTS::TGameBrain TGameController::GameBrain() const
{
    return m_gameBrain;
}

void TGameController::setGameBrain( MODEL_COMPONENTS::TGameBrain _gameBrain )
{
    m_gameBrain = _gameBrain;
}

MODEL_COMPONENTS::TGameStage TGameController::GameStage() const
{
    return PlayerGameProcess_cref( ).GameStage();
}

void TGameController::InitGame()
{
    PlayerGameProcess_ref().Init();
    ComputerGameProcess_ref().Init();
}

void TGameController::CreateGame()
{
    assert( PlayerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );
    assert( ComputerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );
    switch (GameMode())
    {
    case MODEL_COMPONENTS::TGameMode::PLAYER:
        PlayerGameProcess_ref().setTrueGameValue( TStandartRules::Instance()->GetRandomGameValue(PlayerGameProcess_ref().RandomByModulus()) );
        break;
    case MODEL_COMPONENTS::TGameMode::COMPUTER:
        ComputerGameProcess_ref().setTrueGameValue( TStandartRules::Instance()->GetRandomGameValue(ComputerGameProcess_ref().RandomByModulus()) );
        break;
    case MODEL_COMPONENTS::TGameMode::PLAYER_VS_COMPUTER:
        {
            auto randomValue = TStandartRules::Instance()->GetRandomGameValue(PlayerGameProcess_ref().RandomByModulus());
            PlayerGameProcess_ref().setTrueGameValue( randomValue );
            ComputerGameProcess_ref().setTrueGameValue( randomValue );
        }
        break;
    default:
        assert(false && "wrong game mode!");
        break;
    }
    emit gameStarted(GameMode());
}

void TGameController::PlayerStep( std::vector< uint8_t > const & _gameValueList )
{
    assert( PlayerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS );
    PlayerGameProcess_ref().appendGameValue(TGameValue(_gameValueList));

    sendProcessResults(PlayerGameProcess_ref(), MODEL_COMPONENTS::TGameMode::PLAYER);
    if(
        GameMode() == MODEL_COMPONENTS::TGameMode::PLAYER_VS_COMPUTER &&
        ComputerGameProcess_ref().GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED
    )
    {
        makeStep();
    }

    if( PlayerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED )
    {
        while(ComputerGameProcess_ref().GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED)
        {
            makeStep();
        }
        if(PlayerGameProcess_ref( ).AttemptsCount() < ComputerGameProcess_ref().AttemptsCount())
        {
            emit gameFinished( MODEL_COMPONENTS::TGameMode::PLAYER );
        }
        if(PlayerGameProcess_ref( ).AttemptsCount() == ComputerGameProcess_ref().AttemptsCount())
        {
            emit gameFinished( MODEL_COMPONENTS::TGameMode::PLAYER_VS_COMPUTER );
        }
        if(PlayerGameProcess_ref( ).AttemptsCount() > ComputerGameProcess_ref().AttemptsCount())
        {
            emit gameFinished( MODEL_COMPONENTS::TGameMode::COMPUTER );
        }
    }
}

void TGameController::makeStep()
{
    assert( ComputerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS );
    ComputerGameProcess_ref().makeStep();
    sendProcessResults(ComputerGameProcess_ref(), MODEL_COMPONENTS::TGameMode::COMPUTER);
    if(
        GameMode() == MODEL_COMPONENTS::TGameMode::COMPUTER &&
        ComputerGameProcess_ref().GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED
    )
    {
        emit gameFinished( MODEL_COMPONENTS::TGameMode::COMPUTER );
    }
}

void TGameController::sendProcessResults(TStandartGameProcess const & _gameProcess, MODEL_COMPONENTS::TGameMode _gameMode  )
{
    emit sendAttemptResults(
                _gameMode,
                _gameProcess.HistoryList().back().first.List(),
                _gameProcess.HistoryList().back().second.first,
                _gameProcess.HistoryList().back().second.second,
                _gameProcess.AttemptsCount()
            );
}

void TGameController::gameModeSwitched(MODEL_COMPONENTS::TGameMode _gameMode)
{
    assert( PlayerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );
    assert( ComputerGameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );
    setGameMode(_gameMode);
}

void TGameController::gameBrainSwitched(MODEL_COMPONENTS::TGameBrain _gameBrain)
{
    if(GameBrain() != _gameBrain)
    {
        ComputerGameProcess_ref( ).selectBrain(_gameBrain);
        setGameBrain(_gameBrain);
    }
}
