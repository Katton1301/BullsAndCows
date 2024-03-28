#include "./views/simulation_view.h"

#include <QtGui>
#include <QApplication>

int main ( int argc, char *argv[ ] )
{
    QApplication a(argc, argv);
    TSimulationView w;

    w.OnInitialUpdate( );

    w.show();
    int result = a.exec();
    return result;
}
