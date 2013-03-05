#include "portopencvqt.h"

#include <QtGui>

PortOpenCVQt::PortOpenCVQt(){}

bool PortOpenCVQt::cvtFormat(const cv::Mat & from, QPixmap & to)
{
	if (from.empty())
	{
		qDebug() << "CameraItem::matToPixmap: Input-Image is empty!";
		return false;
	}

	to = QPixmap::fromImage(QImage((const uchar *)(from.data),
								   from.cols,
								   from.rows,
								   QImage::Format_RGB888));
	return true;
}

bool PortOpenCVQt::resize(QPixmap & image,
						int width, int height,
						Qt::TransformationMode transformMode)
{
	if (width <= 0 && height <= 0)
	{
		qDebug() << "CameraItem::resize: new Size is (0,0)!";
		return false;
	}
	else if (width <= 0)
	{
		image = image.scaledToHeight(200);
	}
	else if (height <= 0)
	{
		image = image.scaledToWidth(200);
	}
	else // width > 0 && height > 0
	{
		/*
		image = image.scaled(QSize(width,heigth),
							 Qt::IgnoreAspectRatio,
							 transformMode);
		*/
	}
	return true;
}
