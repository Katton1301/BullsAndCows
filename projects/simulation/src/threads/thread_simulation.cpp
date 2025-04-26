#include <threads/thread_simulation.h>
#include <fstream>

TSimulationThread::TSimulationThread()
{
    m_profilerPlayerProcess.setName( "Game Processor." );
    m_profilerGameStep.setName( "Step Processor." );

    std::random_device device;
    random_generator_.seed(device());

    m_randomByModulus =
        [this]( uint32_t _modulus )->unsigned int
    {
        std::uniform_int_distribution<uint32_t> range(0, _modulus - 1);
        return range(random_generator_);
    };
    m_player_process = std::make_shared<TStandartPlayerProcess>(m_randomByModulus);
}

TSimulationThread::~TSimulationThread()
{
}

void TSimulationThread::Prepare( MODEL_COMPONENTS::TGameBrain _gameBrain, uint64_t _startAmount )
{
    m_statisticAttempts.clear();
    if(m_gameBrain != _gameBrain)
    {
        PlayerProcess_ref( ).selectBrain(_gameBrain);
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

    auto printProfiles = [this]()
    {
        std::ofstream reportProfilers( "profilers.report" );
        reportProfilers << m_profilerPlayerProcess << std::endl;
        reportProfilers << m_profilerGameStep << std::endl;
        reportProfilers.close( );
    };

    m_profilerPlayerProcess.init( );
    m_profilerGameStep.init( );

    for(uint32_t i = 0; i < m_startsAmount; ++i)
    {
        if(m_emergency_stop)
        {
            m_emergency_stop = false;
            break;
        }
        m_profilerPlayerProcess.start();
        PlayerProcess_ref( ).Init();
        assert( PlayerProcess_ref( ).PlayerState() == MODEL_COMPONENTS::TPlayerState::WAIT_A_NUMBER );

        PlayerProcess_ref().setTrueGameValue( TStandartRules::Instance().GetRandomGameValue(PlayerProcess_ref().GetRandom()) );

        while(PlayerProcess_ref( ).PlayerState() != MODEL_COMPONENTS::TPlayerState::FINISHED)
        {
            assert( PlayerProcess_ref( ).PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS );
            m_profilerGameStep.start();
            PlayerProcess_ref().makeStep();
            m_profilerGameStep.stop();
        }

        m_statisticAttempts.try_emplace(PlayerProcess_ref().AttemptsCount(),0);
        ++m_statisticAttempts.at(PlayerProcess_ref().AttemptsCount());


        if ( ( ( i + 1 ) % stepProgressBar ) == 0 )
        {
            auto iProgressValue = static_cast< int >(
                static_cast< double >( i + 1 )
                /  m_startsAmount * 100.0
            );
            emit UpdateProgressBar( iProgressValue );

            printProfiles( );
        }
        m_profilerPlayerProcess.stop();
    }

    printProfiles( );
    emit SimulationFinished();
}
