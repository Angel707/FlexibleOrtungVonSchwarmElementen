#include <QtGui/QApplication>
#include "camerasetting.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CameraSetting w;
    w.show();

    return a.exec();
}
