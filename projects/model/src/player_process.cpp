#include <player_process.hpp>

MODEL_COMPONENTS::TPlayerState TPlayerProcessBase::PlayerState() const
{
    return m_playerState;
}

void TPlayerProcessBase::setPlayerState( MODEL_COMPONENTS::TPlayerState _playerState )
{
    m_playerState = _playerState;
}

bool TPlayerProcessBase::isBrainless( ) const
{
    return true;
}

void TPlayerProcessBase::Init()
{
    setPlayerState(MODEL_COMPONENTS::TPlayerState::WAIT_A_NUMBER);
}

TStandartPlayerProcess::TStandartPlayerProcess( std::function< uint32_t( uint32_t ) > & _randomFunc )
    : TPlayerProcessBase()
    , m_randomFunc(_randomFunc)
    , m_historyList()
    , m_trueGameValue(nullptr)
    , m_gameBrain(nullptr)
{

}

void TStandartPlayerProcess::Init()
{
    TPlayerProcessBase::Init();
    if(GameBrain_ptr())
    {
        GameBrain_ptr()->Init();
    }
    m_historyList.clear();
}

std::shared_ptr<TStandartBrain> TStandartPlayerProcess::GameBrain_ptr()
{
    return m_gameBrain;
}

void TStandartPlayerProcess::selectBrain( MODEL_COMPONENTS::TGameBrain _gameBrain )
{
    assert(PlayerState() != MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
    m_gameBrain = createStandartBrain( this, _gameBrain);
}

TStandartPlayerProcess::THistoryList const & TStandartPlayerProcess::HistoryList( ) const
{
    return m_historyList;
}

uint32_t TStandartPlayerProcess::AttemptsCount( ) const
{
    return m_historyList.size();
}

void TStandartPlayerProcess::setTrueGameValue( TGameValue<uint8_t> const & _gameValue )
{
    assert(PlayerState() == MODEL_COMPONENTS::TPlayerState::WAIT_A_NUMBER);
    if(TStandartRules::Instance().isValidGameValue(_gameValue))
    {
        m_trueGameValue = std::make_shared<TGameValue<uint8_t>>(_gameValue);
        setPlayerState(MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
    }
}

void TStandartPlayerProcess::appendGameValue( TGameValue<uint8_t> const & _gameValue )
{
    assert(PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
    assert(TStandartRules::Instance().isValidGameValue(_gameValue));
    auto bullsNCows = TStandartRules::Instance().calculateBullsAndCows(_gameValue, *m_trueGameValue);
    m_historyList.push_back(std::make_pair(_gameValue,bullsNCows));
    if(TStandartRules::Instance().isWinResults(bullsNCows))
    {
        setPlayerState(MODEL_COMPONENTS::TPlayerState::FINISHED);
    }
}

void TStandartPlayerProcess::makeStep( )
{
    assert(PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
    assert(GameBrain_ptr());
    GameBrain_ptr()->makePredict();
    assert(GameBrain_ptr()->PredictedValue());
    appendGameValue(*GameBrain_ptr()->PredictedValue());
    if(TStandartRules::Instance().isWinResults(HistoryList().back().second))
    {
        setPlayerState(MODEL_COMPONENTS::TPlayerState::FINISHED);
    }
}


void TStandartPlayerProcess::giveUp( )
{
    assert(PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
    setPlayerState(MODEL_COMPONENTS::TPlayerState::GAVE_UP);
}

std::function< uint32_t( uint32_t ) > const & TStandartPlayerProcess::GetRandom() const
{
    return m_randomFunc;
}
