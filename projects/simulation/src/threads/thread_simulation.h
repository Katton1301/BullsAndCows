#pragma once
#include "core/enums.hpp"
#include "game_process.hpp"
#include "tools/time_profiler.hpp"

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

signals :
    void UpdateProgressBar( int in_iValue );
    void SimulationFinished( );

private:
    void handleResults();
    TStandartGameProcess & GameProcess_ref( )
    {
        return *m_game_process;
    }
    TStandartGameProcess & GameProcess_cref( ) const
    {
        return *m_game_process;
    }

private:
    std::shared_ptr<TStandartGameProcess> m_game_process{};
    MODEL_COMPONENTS::TGameBrain m_gameBrain = MODEL_COMPONENTS::TGameBrain::UNKNOWN;
    uint64_t m_startsAmount;
    std::map<uint32_t, uint64_t> m_statisticAttempts{};

    TTimeProfiler m_profilerGameProcess;
    TTimeProfiler m_profilerGameStep;

};
