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

            default :
                stream_ << "[Error] identifier of game mode is wrong : " << static_cast< int32_t >( _gameMode ) << std::endl;
                assert( false && " incorrect game mode identifier" );
                break;
        }
        return stream_;
    }
}


TGameController::TGameController()
    : m_game_process(std::make_shared<TStandartGameProcess>())
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
    return GameProcess_cref( ).GameStage();
}

void TGameController::InitGame()
{
    GameProcess_ref().Init();
}

void TGameController::CreateGame()
{
    assert( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );

    GameProcess_ref().setTrueGameValue( TStandartRules::Instance()->GetRandomGameValue(GameProcess_ref().RandomByModulus()) );
    emit gameStarted(GameMode());
}

void TGameController::appendGameValue( std::vector< uint8_t > const & _gameValueList )
{
    assert( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS );
    assert( GameMode() == MODEL_COMPONENTS::TGameMode::PLAYER );
    GameProcess_ref().appendGameValue(TGameValue(_gameValueList));
    handleResults();
}

void TGameController::makeStep()
{
    assert( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS );
    assert( GameMode() == MODEL_COMPONENTS::TGameMode::COMPUTER );
    GameProcess_ref().makeStep();
    handleResults();
}

void TGameController::handleResults()
{
    emit sendAttemptResults(
                GameProcess_ref().HistoryList().back().first.List(),
                GameProcess_ref().HistoryList().back().second.first,
                GameProcess_ref().HistoryList().back().second.second,
                GameProcess_ref().AttemptsCount()
            );
    if( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED )
    {
        emit gameFinished( GameMode() );
    }
}

void TGameController::gameModeSwitched(MODEL_COMPONENTS::TGameMode _gameMode)
{
    assert( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );
    setGameMode(_gameMode);
}

void TGameController::gameBrainSwitched(MODEL_COMPONENTS::TGameBrain _gameBrain)
{
    if(GameBrain() != _gameBrain)
    {
        GameProcess_ref( ).selectBrain(_gameBrain);
        setGameBrain(_gameBrain);
    }
}
