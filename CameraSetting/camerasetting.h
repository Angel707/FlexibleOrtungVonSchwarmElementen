#ifndef CAMERASETTING_H
#define CAMERASETTING_H

#include <QDialog>
#include <opencv2/opencv.hpp>
#include "../Camera/cvlibwrapper.h"

namespace Ui {
	class CameraSetting;
}

class CameraSetting : public QWidget
{
	Q_OBJECT

public:
	explicit CameraSetting(QWidget *parent = 0);
	~CameraSetting();

	void getPoints(
		std::vector<cv::Point2f> & fourPixelPoints,
		std::vector<cv::Point2f> & fourObjectPoints
		);

	void getProjection(
		cv::Mat_<float> & pixelToWorld_Projection,
		cv::Mat_<float> & worldToPixel_Projection
		);

signals:
	void confirmed();

private slots:
	void on_btnClose_clicked();

private:
	Ui::CameraSetting *ui;
};

#endif // CAMERASETTING_H
