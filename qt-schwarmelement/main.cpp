#include <QtGui/QApplication>
#include "schwarmelementwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SchwarmElementWidget w;
    w.show();

    return a.exec();
}
