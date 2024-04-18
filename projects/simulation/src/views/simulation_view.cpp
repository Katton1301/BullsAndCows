#include "simulation_view.h"
#include "common_operations.hpp"
#include <QtWidgets/QMessageBox>

QTableWidgetItem * createTableItem(QString text)
{
    auto item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignHCenter);
    return item;
}

TSimulationView::TSimulationView( QWidget* _parent )
    : QWidget( _parent )
    , m_threadSimulation( )
{
    ui.setupUi( this );
    ui.resultsTable->setColumnCount(2);
    ui.resultsTable->setHorizontalHeaderItem(0, createTableItem("Moves Amount"));
    ui.resultsTable->setHorizontalHeaderItem(1, createTableItem("Probability"));
    ui.resultsTable->clearContents();
    ui.resultsTable->setRowCount(0);
    ui.resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.resultFrame->setVisible(false);

    QObject::connect(
        &m_threadSimulation,
        SIGNAL( UpdateProgressBar(int) ),
        this,
        SLOT( onUpdateStatistic(int) )
    );

    QObject::connect(
        &m_threadSimulation,
        SIGNAL( SimulationFinished() ),
        this,
        SLOT  ( onSimulateFinish() )
    );

}

TSimulationView::~TSimulationView( )
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;

    if ( m_threadSimulation.isRunning( ) )
    {
        std::cout << "The Simulation Thread is running .... kill here";
        m_threadSimulation.terminate( );

        if ( !m_threadSimulation.isFinished( ) )
        {
            std::cout << " ... await";
            m_threadSimulation.wait( 1500 );
        }
        else
        {
            std::cout << " ... OK";
        }

         std::cout << std::endl;
    }
}

void TSimulationView::OnInitialUpdate( )
{
    ui.gamesAmountEdit->setText( QString( "500000000" ) );
    for(auto const & gameBrain : generateGameBrainsList())
    {
        ui.brainComboBox->addItem(gameBrain);
    }
}

void TSimulationView::onUpdateStatistic( int percent )
{
    ui.progressBar->setValue( percent );
    fillStatisticTable(percent);
}

void TSimulationView::onSimulationButton( )
{
    if ( m_threadSimulation.isRunning( ) )
    {
        ui.startButton->setText( "Start" );
        stopSimulate();
    }
    else
    {
        ui.startButton->setText( "Stop" );
        startSimulate();
    }
}

void TSimulationView::startSimulate( )
{
    std::cout << "===> " << __PRETTY_FUNCTION__ << std::endl;

    ui.resultsTable->clearContents();
    ui.resultsTable->setRowCount(0);
    ui.resultFrame->setVisible(false);

    MODEL_COMPONENTS::TGameBrain gameBrain = COMMON_OPERATIONS::convertFromStdString(
                    ui.brainComboBox->currentText().toStdString(),
                    MODEL_COMPONENTS::TGameBrain::BEGIN,
                    MODEL_COMPONENTS::TGameBrain::BEGIN,
                    MODEL_COMPONENTS::TGameBrain::END
                );

    ui.progressBar->setValue( 0 );

    m_threadSimulation.Prepare( gameBrain, ui.gamesAmountEdit->text().toUInt() );
    m_threadSimulation.start( );


    std::cout << "Simulation Thread isRunning( ) : " << m_threadSimulation.isRunning( ) << std::endl;
    std::cout << "Simulation Thread isFinished( ) : " << m_threadSimulation.isFinished( ) << std::endl;
    std::cout << __PRETTY_FUNCTION__  << " ===>" << std::endl;
}

void TSimulationView::stopSimulate( )
{
    std::cout << "===> " << __PRETTY_FUNCTION__ << std::endl;
    m_threadSimulation.terminate( );

    std::cout << "Simulation Thread isRunning( ) : " << m_threadSimulation.isRunning( ) << std::endl;
    std::cout << "Simulation Thread isFinished( ) : " << m_threadSimulation.isFinished( ) << std::endl;

    std::cout << __PRETTY_FUNCTION__  << " ===>" << std::endl;
}

void TSimulationView::onSimulateFinish( )
{
    std::cout << "===> " << __PRETTY_FUNCTION__ << std::endl;
    fillStatisticTable(100);
    m_threadSimulation.wait( 1000 );

    QMessageBox messageOk;
    messageOk.setText( QString( "The Simulation was finished." ) );
    messageOk.exec();

    ui.startButton->setText( "Start" );
    std::cout << __PRETTY_FUNCTION__  << " ===>" << std::endl;
}

void TSimulationView::fillStatisticTable( int percent )
{
    ui.resultsTable->clearContents();
    ui.resultsTable->setRowCount(0);
    uint64_t movesAmountSumm = 0;
    uint64_t gamesAmount = m_threadSimulation.StartsAmount() * percent / 100;
    for( auto const & item : m_threadSimulation.StatisticAttempts())
    {
        ui.resultsTable->setRowCount(ui.resultsTable->rowCount() + 1);
        ui.resultsTable->setItem(ui.resultsTable->rowCount() - 1, 0, createTableItem(QString("%1").arg(item.first)));
        ui.resultsTable->setItem(ui.resultsTable->rowCount() - 1, 1, createTableItem(QString("%1").arg(
            static_cast<double>(item.second) / gamesAmount
        )));
        movesAmountSumm += item.first * item.second;
    }
    ui.avgMovesAmount->setText(QString("%1").arg(static_cast<double>(movesAmountSumm)/gamesAmount));
    ui.resultFrame->setVisible(true);
}

std::vector<QString> TSimulationView::generateGameBrainsList()  const
{
    std::vector<QString> gameBrainsList;
    for(
        uint32_t gameBrainId = static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::BEGIN);
        gameBrainId < static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::END);
        ++gameBrainId
    )
    {
        auto gameBrainStr = COMMON_OPERATIONS::convertToStdString(static_cast<MODEL_COMPONENTS::TGameBrain>(gameBrainId));
        gameBrainsList.push_back(QString(gameBrainStr.c_str()));
    }
    return gameBrainsList;
}
