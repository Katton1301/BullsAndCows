#include "./views/game_view.h"

#include <QtGui>
#include <QApplication>

int main ( int argc, char *argv[ ] )
{
    QApplication a(argc, argv);
    TGameView w;

    w.OnInitialUpdate( );

    w.show();
    int result = a.exec();
    return result;
}
