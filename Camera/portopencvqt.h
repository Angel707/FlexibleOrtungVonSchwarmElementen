#ifndef PORTOPENCVQT_H
#define PORTOPENCVQT_H

#include <opencv2/opencv.hpp>
#include <QtGui>

class PortOpenCVQt
{
public:
	PortOpenCVQt();
	static bool cvtFormat(const cv::Mat & from, QPixmap & to);
	static bool resize(QPixmap & image,
						int width, int height,
						Qt::TransformationMode transformMode = Qt::FastTransformation);
	void debug();

};

#endif // PORTOPENCVQT_H
