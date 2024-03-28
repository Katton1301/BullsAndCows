#include "thread_simulation.h"

TSimulationThread::TSimulationThread()
    : m_game_process(std::make_shared<TStandartGameProcess>())
    , m_gameBrain(MODEL_COMPONENTS::TGameBrain::UNKNOWN)
    , m_startsAmount(0)
    , m_statisticAttempts()
{
}

TSimulationThread::~TSimulationThread()
{

}

void TSimulationThread::Prepare( MODEL_COMPONENTS::TGameBrain _gameBrain, uint64_t _startAmount )
{
    m_statisticAttempts.clear();
    if(m_gameBrain != _gameBrain)
    {
        GameProcess_ref( ).selectBrain(_gameBrain);
        m_gameBrain = _gameBrain;
    }
    m_startsAmount = _startAmount;
}

void TSimulationThread::run( )
{
    uint32_t stepProgressBar = m_startsAmount / 100;
    if ( stepProgressBar == 0 )
    {
        stepProgressBar =
            100
            / m_startsAmount
        ;
    }

    for(uint32_t i = 0; i < m_startsAmount; ++i)
    {
        GameProcess_ref( ).Init();
        assert( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER );

        GameProcess_ref().setTrueGameValue( TStandartRules::Instance()->GetRandomGameValue(GameProcess_ref().RandomByModulus()) );

        while(GameProcess_ref( ).GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED)
        {
            assert( GameProcess_ref( ).GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS );
            GameProcess_ref().makeStep();
        }
        if(!m_statisticAttempts.contains(GameProcess_ref().AttemptsCount()))
        {
            m_statisticAttempts.emplace(GameProcess_ref().AttemptsCount(),0);
        }
        ++m_statisticAttempts.at(GameProcess_ref().AttemptsCount());


        if ( ( ( i + 1 ) % stepProgressBar ) == 0 )
        {
            auto iProgressValue = static_cast< int >(
                static_cast< double >( i + 1 )
                /  m_startsAmount * 100.0
            );
            emit UpdateProgressBar( iProgressValue );
        }
    }
    emit SimulationFinished();
}
