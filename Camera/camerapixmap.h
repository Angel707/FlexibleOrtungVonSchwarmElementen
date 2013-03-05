#ifndef CAMERAPIXMAP_H
#define CAMERAPIXMAP_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include "cvlibwrapper.h"
#include "Camera.h"

class CameraPixmap : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
public: //=====================================================================
	//-------------------------------------------------------------------------
	// internen Eigenschaften (Name, etc.)
	//-------------------------------------------------------------------------
	explicit CameraPixmap(Camera * cam, QGraphicsItem *parent = 0);
	Camera * camera() const;
	void setScene(QGraphicsScene & scene);
	//-------------------------------------------------------------------------
	// Anzeige des Pixmaps
	//-------------------------------------------------------------------------
	void setPixmap(const QGraphicsPixmapItem * item);
	bool setPixmap(
		const QPixmap & pixmapToBeScaled, bool ignoreChangesInWidthOrHeight = false);
	//-------------------------------------------------------------------------
	// Skalierung des Pixmaps
	//-------------------------------------------------------------------------
	qreal getOldHeight() const;
	qreal getOldWidth() const;
	qreal getNewHeight() const;
	qreal getNewWidth() const;
	qreal getHeightScale() const;
	qreal getWidthScale() const;
	void removeWidthScale();
	void removeHeightScale();
	void setWidthScale(qreal oldWidth, qreal newWidth);
	void setHeightScale(qreal oldHeight, qreal newHeight);
	//-------------------------------------------------------------------------
	// Events
	//-------------------------------------------------------------------------
	//bool execMouseMoveEvent();
	void execMouseReleaseEvent();
	bool handleMouseMoveEvent(const QMouseEvent & event, const QGraphicsView & view);
	bool handleMouseReleaseEvent(const QMouseEvent & event, const QGraphicsView & view);
	static CameraPixmap * isCameraPixmap(const QMouseEvent & event, const QGraphicsView & view);

signals: //====================================================================
	void clicked();
	void switchPixmapRetriever(CameraPixmap * item);

public slots: //===============================================================
	void slotClicked();
	void setUndistortedImage(Camera * cam);
	void setAnalyseImageBGR(cv::Mat imageBGR);
	void setAnalyseImageGray(cv::Mat imageGray);

protected: //==================================================================


private: //====================================================================

	//! Initialisiert ein paar Objekteigenschaften
	void init();

	//! Pixmap zeigt auf eine bestimmte Kamera.
	//! Von dieser werden die Koordinaten geholt.
	Camera * cam;

	// Scaling -> Width
	bool useWidthScaling;
	qreal oldWidth;
	qreal newWidth;
	qreal widthScale;

	// Scaling -> Height
	bool useHeightScaling;
	qreal oldHeight;
	qreal newHeight;
	qreal heightScale;
};

#endif // CAMERAPIXMAP_H
