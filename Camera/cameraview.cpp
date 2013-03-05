#include "cameraview.h"

#include <QtDebug>
#include <assert.h>

CameraView::CameraView(QWidget * parent) :
	QGraphicsView(parent)
{
	// Interne Eigenschaften
	scene = new QGraphicsScene();
	assert(scene);

	margin = 5;
	cam_width = 640;
	cam_maxheigth = 0;
	extrinsicCameraParamDetected = false;

	// Vererbte Eigenschaften
	this->setScene(scene);
}

void CameraView::mouseMoveEvent(QMouseEvent * mouseEvent)
{
	mouseEvent->accept();
	qDebug() << ">>>" << this->objectName() << "mouseMoveEvent:" << "accept.";
	CameraPixmap * item = CameraPixmap::isCameraPixmap(*mouseEvent, *this);
	if (item)
		item->handleMouseMoveEvent(*mouseEvent, *this);
}

void CameraView::mouseReleaseEvent(QMouseEvent * mouseEvent)
{
	mouseEvent->accept();
	qDebug() << ">>>" << this->objectName() << "mouseReleaseEvent:" << "accept.";
	CameraPixmap * item = CameraPixmap::isCameraPixmap(*mouseEvent, *this);
	if (item)
		item->handleMouseReleaseEvent(*mouseEvent, *this);
}

void CameraView::setCameraWidth(int newWidth)
{
	cam_width = newWidth;
}

bool CameraView::addCamera(Camera * cam, SchwarmElementDetection * sed)
{
	assert(cam);
	assert(sed);
	// Start Recording
	if (!cam->activateDevice())
	{
		qCritical() << "!!" << this->objectName() << "addCamera:"
					<< "could not activate Camera!";
		return false;
	}
	cam->recordingStart();

	// First Test Image
	if (!cam->recordingUpdate_Full())
	{
		qCritical() << "!!" << this->objectName() << "addCamera:"
					<< "could not recorde!";
		return false;
	}
	QPixmap image;
	if (!cam->getRecordedUndistortedPixmap(image))
	{
		qCritical() << "!!" << this->objectName() << "Add Camera:"
					<< "could not load (undistorted) image!";
		return false;
	}

	qDebug() << "-" << this->objectName() << "Add Camera:"
			 << "Camera" << cam->objectName() << "erfolgreich initialisiert.";

	// Add to Scene
	QGraphicsItemGroup * camItem = new QGraphicsItemGroup();
	if (!camItem) return false;
	QGraphicsTextItem * textItem1 = scene->addText(cam->objectName());
	if (!textItem1) return false;
	CameraPixmap * pixItem1 = new CameraPixmap(cam);
	if (!pixItem1) return false;

	// Bild-Analyse

	CameraPixmap * pixItemAnalyse = new CameraPixmap(cam);
	if (!pixItemAnalyse) return false;

	qreal item_x = (camList.size() - 1) * cam_width;

	textItem1->setObjectName(cam->objectName());
	textItem1->setTextWidth(cam_width);
	textItem1->setPos(item_x, 0);

	pixItem1->setObjectName(cam->objectName());
	pixItem1->setPos(item_x, 30);
	pixItem1->setWidthScale(image.width(), cam_width);
	pixItem1->setPixmap(image);
	pixItem1->setScene(*scene);

	qDebug() << "-" << "pixItem1-botttom:" << pixItem1->boundingRect().bottom();

	pixItemAnalyse->setObjectName(QString("%1: %2").arg(cam->objectName(), "Analyse"));
	pixItemAnalyse->setPos(item_x, (pixItem1->boundingRect().bottom()+35));
	pixItemAnalyse->setWidthScale(image.width(), cam_width);
	pixItemAnalyse->setPixmap(image);
	pixItemAnalyse->setScene(*scene);

	this->setInteractive(true);
	this->setMouseTracking(true);

	camItem->addToGroup(textItem1); // Index 0
	camItem->addToGroup(pixItem1);  // Index 1
	camItem->addToGroup(pixItemAnalyse);  // Index 2

	scene->addItem(camItem);

	qDebug() << "-" << this->objectName() << "Add Camera:"
			 << "Scene initialisiert.";

	camItem->show();
	cam->setting()->show();

	// Signal an Kamera, wenn recordingUpdate fertig ist
	connect(this, SIGNAL(cameraGrabFinished()),
			cam, SLOT(recordingAllGrabsFinished()));

	// Erhalte unbearbeitete und entzerrte Kamera-Bilder
	connect(cam, SIGNAL(newImage(Camera*)),
			pixItem1, SLOT(setUndistortedImage(Camera*)));

	// Erhalte Analyse-Bilder
	connect(sed, SIGNAL(currentObjects(cv::Mat)),
			pixItemAnalyse, SLOT(setAnalyseImageBGR(cv::Mat)));

	// Zum Wechseln des Analysers
	connect(pixItemAnalyse, SIGNAL(switchPixmapRetriever(CameraPixmap*)),
			this, SLOT(switchAnalyser(CameraPixmap*)));

	// Camera management
	camItemList.push_back(camItem);
	camList.push_back(cam);
	detectList.push_back(sed);
	detectFunction.push_back(0);
	qDebug() << "-" << this->objectName() << "addCamera:" << "Liste aktualisiert.";
	return true;
}

void CameraView::recordingUpdate()
{
	assert(camList.size() == camItemList.size());
	// Grab Images
	for (int i = 0; i < camList.size(); i++)
	{
		Camera * cam = dynamic_cast<Camera *>(camList[i]);
		assert(cam);
		if (!cam->recordingUpdate_Grab())
		{
			qCritical() << "!!" << this->objectName() << "recordingUpdate:" << "Cannot grab from Camera" << i;
		}
	}
	qDebug() << "<(signal)<" << this->objectName() << "recordingUpdate:"
			 << "Alle Kamera-Bilder aktualisiert (grab)..";
	emit cameraGrabFinished();
	/*
	// DIESER TEIL WIRD NICHT MEHR GEBRAUCHT, da sich die einzelnen Pixmaps
	// die Bilder selbst holen. Dies geschieht durch das Signal-Slot-System
	// von Qt! So kann ein Pixmap entscheiden, von wo es sich die Daten holt.
	// Es wird insbesondere zur Analyse der Kamera-Bilder benutzt, da
	// diese nicht vom Objekt Camera stammen, sondern von SchwarmElementDetection.

	// Retrieve Images
	for (unsigned int i = 0; i < camList.size(); i++)
	{
		Camera * cam = dynamic_cast<Camera *>(camList[i]);
		assert(cam);
		if (!cam->recordingUpdate_RetrieveImage())
		{
			qCritical() << "!!" << this->objectName() << "recordingUpdate:" << "Cannot retrieve Image from Camera" << i;
			continue;
		}
		QPixmap image;
		if (!cam->getRecordedUndistortedPixmap(image))
		{
			qCritical() << "!!" << this->objectName() << "recordingUpdate:" << "could not load (undistorted) image from Camera" << i;
			continue;
		}
		QGraphicsItemGroup * gItem = dynamic_cast<QGraphicsItemGroup *>(camItemList[i]);
		CameraPixmap * pItem = qgraphicsitem_cast<CameraPixmap *>(gItem->childItems().at(1));
		pItem->setPixmap(image);
	}
	*/
}

// SLOT
void CameraView::switchAnalyser(CameraPixmap * item)
{
	assert(item);
	qDebug() << ">(slot)>" << this->objectName() << "switchAnalyser:"
			 << "Wechsel das Analyse-Bild fuer" << item->objectName() << "...";
	assert(item->camera());
	// Es werden nur Pixmaps erstellt, wenn gültige Kameras vorhanden sind.
	// Daher muss eine Lösung existieren
	Camera * curr = NULL;
	int i = 0;
	for (; i < camList.size(); i++)
	{
		// Suche die richtige Kamera in der Liste
		curr = camList[i];
		if (curr == item->camera())
		{
			// gefunden
			break;
		}
	}
	assert(i < camList.size());

	// Der richtige Analyser
	SchwarmElementDetection * sed = detectList[i];

	// Disconnect
	item->disconnect(sed, NULL, item, NULL);

	// Counter erhöhen
	detectFunction[i]++;
	detectFunction[i] %= 6;

	// Connect
	switch(detectFunction[i])
	{
	case 1: connect(sed, SIGNAL(currentBlur(cv::Mat)),
					item, SLOT(setAnalyseImageBGR(cv::Mat)));
		item->setToolTip("1. Gaussian Blur");
		break;
	case 2: connect(sed, SIGNAL(currentColorReduced(cv::Mat)),
					item, SLOT(setAnalyseImageBGR(cv::Mat)));
		item->setToolTip("2. Color Reduced");
		break;
	case 3: connect(sed, SIGNAL(currentBackProjection(cv::Mat)),
					item, SLOT(setAnalyseImageGray(cv::Mat)));
		item->setToolTip("3. Hue Back Projection");
		break;
	case 4: connect(sed, SIGNAL(currentSegmentation(cv::Mat)),
					item, SLOT(setAnalyseImageGray(cv::Mat)));
		item->setToolTip("4. Segmentation");
		break;
	case 5: connect(sed, SIGNAL(currentWatershed(cv::Mat)),
					item, SLOT(setAnalyseImageGray(cv::Mat)));
		item->setToolTip("4. Watershed");
		break;
	default: connect(sed, SIGNAL(currentObjects(cv::Mat)),
					item, SLOT(setAnalyseImageBGR(cv::Mat)));
		item->setToolTip("5. Objects");
	}
}
