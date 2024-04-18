#include "game_process.hpp"

MODEL_COMPONENTS::TGameStage TGameProcessBase::GameStage() const
{
    return m_gameStage;
}

void TGameProcessBase::setGameStage( MODEL_COMPONENTS::TGameStage _gameStage )
{
    m_gameStage = _gameStage;
}

void TGameProcessBase::Init()
{
    setGameStage(MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER);
}

TStandartGameProcess::TStandartGameProcess( )
    : TGameProcessBase()
    , m_historyList()
    , m_trueGameValue(nullptr)
    , m_gameBrain(nullptr)
{
    std::random_device device;
    random_generator_.seed(device());
}

void TStandartGameProcess::Init()
{
    TGameProcessBase::Init();
    m_randomByModulus =
        [this]( uint32_t _modulus )->unsigned int
        {
            std::uniform_int_distribution<uint32_t> range(0, _modulus - 1);
            return range(random_generator_);
        };
    m_historyList.clear();
}

std::shared_ptr<TStandartBrain> TStandartGameProcess::GameBrain_ptr()
{
    return m_gameBrain;
}

void TStandartGameProcess::selectBrain( MODEL_COMPONENTS::TGameBrain _gameBrain )
{
    m_gameBrain = createStandartBrain( this, _gameBrain);
}

TStandartGameProcess::THistoryList const & TStandartGameProcess::HistoryList( ) const
{
    return m_historyList;
}

uint32_t TStandartGameProcess::AttemptsCount( ) const
{
    return m_historyList.size();
}

void TStandartGameProcess::setTrueGameValue( TGameValue<uint8_t> const & _gameValue )
{
    assert(GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER);
    if(TStandartRules::Instance()->isValidGameValue(_gameValue))
    {
        m_trueGameValue = new TGameValue(_gameValue);
        setGameStage(MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        m_gameBrain->Init();
    }
}

void TStandartGameProcess::appendGameValue( TGameValue<uint8_t> const & _gameValue )
{
    assert(GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
    assert(TStandartRules::Instance()->isValidGameValue(_gameValue));
    auto bullsNCows = TStandartRules::Instance()->calculateBullsAndCows(_gameValue, *m_trueGameValue);
    m_historyList.push_back(std::make_pair(_gameValue,bullsNCows));
    if(TStandartRules::Instance()->isWinResults(bullsNCows))
    {
        setGameStage(MODEL_COMPONENTS::TGameStage::FINISHED);
    }
}

void TStandartGameProcess::makeStep( )
{
    assert(GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
    assert(GameBrain_ptr());
    GameBrain_ptr()->makePredict();
    assert(GameBrain_ptr()->PredictedValue_cptr());
    appendGameValue(*GameBrain_ptr()->PredictedValue_cptr());
    if(TStandartRules::Instance()->isWinResults(HistoryList().back().second))
    {
        setGameStage(MODEL_COMPONENTS::TGameStage::FINISHED);
    }

}

std::function< uint32_t( uint32_t ) > const & TStandartGameProcess::RandomByModulus() const
{
    return m_randomByModulus;
}
