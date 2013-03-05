#ifndef BIRDSEYEVIEW_H
#define BIRDSEYEVIEW_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include "../Camera/Camera.h"
#include "schwarmelementitem.h"

class BirdsEyeView : public QGraphicsView
{
	Q_OBJECT
public:
	BirdsEyeView(QWidget * parent = 0);
	~BirdsEyeView(){}
	//! Zuerst das Spielfeld initialisieren!
	//! Änderungen des Spielfelds, sobald eine Kamera geladen ist,
	//! sind nicht geplant.
	void initBorder(qreal width, qreal height);
	void recordingUpdate();

	void setNoVisiblePolygon(
		QPolygonF polygon
		);

	bool addItem(SchwarmElementItem * item);

signals:
	void clicked();

public slots:
	void slotClicked();
	void updateItem(QGraphicsItem * item);
	void addNewItem(SchwarmElementItem * item);
	bool isNewItem(const QPoint & itemPos);

protected:
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);

private:
	QGraphicsScene * scene;
	QVector<QGraphicsItem*> objectList;

	QGraphicsRectItem * border; // Angaben in mm
	bool borderMustInit;
};

#endif // BIRDSEYEVIEW_H
