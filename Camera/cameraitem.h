#ifndef CAMERAITEM_H
#define CAMERAITEM_H

#include <opencv2/opencv.hpp>
#include <QGraphicsPixmapItem>

class CameraItem : public QGraphicsPixmapItem
{
	Q_OBJECT
public:
	explicit CameraItem(QObject *parent = 0);

signals:

public slots:

private:
	cv::Mat capture(void);
	bool cvtFormat(const cv::Mat & from, QPixmap & to);
	bool resize();
};

#endif // CAMERAITEM_H
