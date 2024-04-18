#pragma once

#include <iostream>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>

#include "ui_simulation_view.h"
#include <memory>
#include <string>
#include "../threads/thread_simulation.h"

class TSimulationView : public QWidget
{
    Q_OBJECT

public:
    explicit TSimulationView( QWidget* _parent = nullptr );
    ~TSimulationView( ) override;

    void OnInitialUpdate( );

public slots :
    void onSimulationButton( );
    void onUpdateStatistic( int percent );
    void onSimulateFinish( );

private :
//methods
    void startSimulate();
    void stopSimulate();
    void fillStatisticTable( int percent );
    std::vector<QString> generateGameBrainsList() const;

//attributes
    Ui::TFormSimulationView ui{};
    TSimulationThread m_threadSimulation;

};
