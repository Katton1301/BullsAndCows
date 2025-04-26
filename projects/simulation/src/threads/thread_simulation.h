#pragma once
#include <core/enums.hpp>
#include <player_process.hpp>
#include <tools/time_profiler.hpp>
#include <random>
#include <QtCore/QThread>


class TSimulationThread : public QThread
{
    Q_OBJECT
public:
    TSimulationThread();
    ~TSimulationThread() override;

    void Prepare( MODEL_COMPONENTS::TGameBrain _gameBrain, uint64_t _startAmount );
    void run( ) override;

    std::map<uint32_t, uint64_t> StatisticAttempts()
    {
        return m_statisticAttempts;
    }

    uint64_t StartsAmount()
    {
        return m_startsAmount;
    }

    void EmegencyStopRequest()
    {
        m_emergency_stop = true;
    }

signals :
    void UpdateProgressBar( int in_iValue );
    void SimulationFinished( );

private:
    void handleResults();
    TStandartPlayerProcess & PlayerProcess_ref( )
    {
        return *m_player_process;
    }
    TStandartPlayerProcess & PlayerProcess_cref( ) const
    {
        return *m_player_process;
    }

private:
    std::mt19937 random_generator_;
    std::function< uint32_t( uint32_t ) > m_randomByModulus;
    std::shared_ptr<TStandartPlayerProcess> m_player_process{};
    MODEL_COMPONENTS::TGameBrain m_gameBrain = MODEL_COMPONENTS::TGameBrain::NONE;
    uint64_t m_startsAmount = 0;
    std::map<uint32_t, uint64_t> m_statisticAttempts{};

    TTimeProfiler m_profilerPlayerProcess;
    TTimeProfiler m_profilerGameStep;

    bool m_emergency_stop = false;
};
