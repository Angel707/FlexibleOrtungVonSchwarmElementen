#include <QtGui/QApplication>
#include "cameracalibrationwidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CameraCalibrationWidget w;
	w.show();

	return a.exec();
}
