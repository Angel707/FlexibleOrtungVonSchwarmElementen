#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include "Camera.h"
#include "camerapixmap.h"
#include "../CameraSetting/camerasetting.h"
#include "schwarmelementdetection.h"

class CameraView : public QGraphicsView
{
	Q_OBJECT
public:
	CameraView(QWidget * parent = 0);
	~CameraView(){}
	bool addCamera(Camera * cam, SchwarmElementDetection * sed);
	void recordingUpdate();
	void setCameraWidth(int newWidth);

signals:
	//void clicked();
	void cameraGrabFinished();

public slots:
	void switchAnalyser(CameraPixmap * item);

protected:
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);

private:
	QGraphicsScene * scene;
	QVector<Camera*> camList;
	QVector<QGraphicsItemGroup*> camItemList;
	QVector<SchwarmElementDetection*> detectList;
	QVector<uint> detectFunction;
	int margin;
	int cam_width;
	int cam_maxheigth;
	qreal width_scale;
	qreal height_scale;
	bool extrinsicCameraParamDetected;
};

#endif // CAMERAVIEW_H
