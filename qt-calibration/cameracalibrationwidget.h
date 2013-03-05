#ifndef CAMERACALIBRATIONWIDGET_H
#define CAMERACALIBRATIONWIDGET_H

#include <QWidget>
#include <QtGui>
#include "../Camera/calibrationframe.h"
#include "../Camera/Camera.h"

namespace Ui {
	class CameraCalibrationWidget;
}

class CameraCalibrationWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CameraCalibrationWidget(QWidget *parent = 0);
	~CameraCalibrationWidget();

private slots:
	void on_pushButtonChangeDevice_clicked();
	void on_pushButtonCalibration_clicked();
	void on_pushButtonSave_clicked();
	void on_pushButtonLoadCalib_clicked();

private:
	Ui::CameraCalibrationWidget *ui;
	Camera cam;
	cv::Mat current_image;

	int timerID;

	// Buttons
	std::vector<CalibrationFrame*> calibList;
	unsigned int calibNumber;
	unsigned int calibCounter;

	// Functions
	CalibrationFrame * createCalibrationFrame(unsigned int i);

protected:
	void timerEvent(QTimerEvent*);
};

#endif // CAMERACALIBRATIONWIDGET_H
