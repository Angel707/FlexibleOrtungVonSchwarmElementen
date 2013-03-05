#ifndef CALIBRATIONFRAME_H
#define CALIBRATIONFRAME_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include "../Camera/Camera.h"

class CalibrationFrame : public QLabel
{
	Q_OBJECT
public:
	CalibrationFrame(int id, Camera * cam, QWidget * parent = 0);
	~CalibrationFrame(){}
	bool isReadyToCalibrate();
	cv::Mat & getImageCV();
	bool restoreCalibration();

signals:
	void clicked();

public slots:
	void slotClicked();

protected:
	void mouseReleaseEvent(QMouseEvent * event);

private:
	int id;
	bool isCalibrationImage;
	cv::Mat image;
	Camera *cam;

	bool readImage();
	void setPixmapCV(cv::Mat & image);
};

#endif // CALIBRATIONFRAME_H
