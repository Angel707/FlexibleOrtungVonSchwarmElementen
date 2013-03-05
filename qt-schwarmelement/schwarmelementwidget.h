#ifndef SCHWARMELEMENTWIDGET_H
#define SCHWARMELEMENTWIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QtGui>

#include <opencv2/opencv.hpp>
#include "../Camera/Camera.h"
#include "../Camera/CamCalibration.h"
#include "../Camera/cameraview.h"
#include "../Camera/schwarmelementitem.h"
#include "../CameraSetting/camerasetting.h"
#include "../Camera/schwarmelementdetection.h"


namespace Ui {
	class SchwarmElementWidget;
}

class SchwarmElementWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SchwarmElementWidget(QWidget * parent = 0);
	~SchwarmElementWidget();

private slots:
	void on_pushButtonAddCamera_clicked();

private:
	Ui::SchwarmElementWidget *ui;
	int timer;
	bool objectImageTaken, objectImageTaken2, backgoundImageTaken, changed;

	void timerEvent(QTimerEvent*);

	// DEBUG: SchwarmElementItem
	//int counter;
	//SchwarmElementItem * k;
	// DEBUG END

};

#endif // SCHWARMELEMENTWIDGET_H
