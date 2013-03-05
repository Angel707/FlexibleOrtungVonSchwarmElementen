#include "detectobject.h"

#include <QtDebug>
#include <assert.h>

// veraltet
#define pixel(cvImage,row,col,channel) ((cvImage).at<cv::Vec3b>((row),(col))[(channel)])

DetectObject::DetectObject(QWidget * parent) :
	QGraphicsView(parent)
{
	// Interne Eigenschaften
	scene = new QGraphicsScene();
	assert(scene);
	cam_width = 320;
	cam_maxheigth = 280;
	detectObjectFunktion = DO_NOOBJECT;
	object = new SchwarmElement();
	assert(object);
	movingCameraCount = 0;

	// Eigenschaften
	this->setScene(scene);
}

// wichtig!! -> Koordinaten anzeigen
void DetectObject::mouseMoveEvent(QMouseEvent * mouseEvent)
{
	mouseEvent->accept();
	CameraPixmap::handleMouseMoveEvent(*mouseEvent, *this);
}

// wichtig
bool DetectObject::addCamera(Camera * cam)
{
	assert(cam);
	// Start Recording (falls noch nicht gestartet)
	if (!cam->isOpened() && !cam->activateDevice())
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
			 << "Camera erfolgreich initialisiert.";

	// Add to Scene
	camItem = new QGraphicsItemGroup();
	if (!camItem) return false;
	QGraphicsTextItem * textItem1 = scene->addText("Without Object");
	if (!textItem1) return false;
	QGraphicsTextItem * textItem2 = scene->addText("With Object");
	if (!textItem2) return false;
	QGraphicsTextItem * textItem3 = scene->addText("Difference");
	if (!textItem3) return false;
	QGraphicsTextItem * textItem4 = scene->addText("Object Selection");
	if (!textItem4) return false;
	QGraphicsTextItem * textItem5 = scene->addText("Test Bird's View");
	if (!textItem4) return false;
	CameraPixmap * pixItem1 = new CameraPixmap(cam);
	if (!pixItem1) return false;
	CameraPixmap * pixItem2 = new CameraPixmap(cam);
	if (!pixItem1) return false;
	CameraPixmap * pixItem3 = new CameraPixmap(cam);
	if (!pixItem1) return false;
	CameraPixmap * pixItem4 = new CameraPixmap(cam);
	if (!pixItem1) return false;
	CameraPixmap * pixItem5 = new CameraPixmap(cam);
	if (!pixItem1) return false;

	textItem1->setObjectName("backgroundItem");
	textItem2->setObjectName("objectItem");
	textItem3->setObjectName("differenceItem");
	textItem4->setObjectName("objectSelectionItem");
	textItem5->setObjectName("testItem");


	textItem1->setTextWidth(cam_width);
	textItem2->setTextWidth(cam_width);
	textItem3->setTextWidth(cam_width);
	textItem4->setTextWidth(image.width());
	textItem5->setTextWidth(image.width());

	textItem1->setPos(0,               0);
	textItem2->setPos(cam_width+5,     0);
	textItem3->setPos((cam_width+5)*2, 0);
	textItem4->setPos(0,               30+cam_maxheigth+5);
	textItem5->setPos(0,               30+cam_maxheigth+30+480+5);

	pixItem1->setObjectName("backgroundObject");
	pixItem2->setObjectName("objectObject");
	pixItem3->setObjectName("differenceObject");
	pixItem4->setObjectName("objectSelectionObject");
	pixItem5->setObjectName("testObject");

	pixItem1->setPos(0,               30);
	pixItem2->setPos(cam_width+5,     30);
	pixItem3->setPos((cam_width+5)*2, 30);
	pixItem4->setPos(0,               30+cam_maxheigth+30+5);
	pixItem5->setPos(0,               30+cam_maxheigth+30+480+30+5);

	pixItem1->setWidthScale(image.width(), cam_width);
	pixItem2->setWidthScale(image.width(), cam_width);
	pixItem3->setWidthScale(image.width(), cam_width);
	pixItem4->setWidthScale(image.width(), image.width());
	pixItem5->setWidthScale(image.width(), image.width());

	pixItem1->setPixmap(image);
	pixItem2->setPixmap(image);
	pixItem3->setPixmap(image);
	pixItem4->setPixmap(image);
	pixItem5->setPixmap(image);

	pixItem1->setScene(*scene);
	pixItem2->setScene(*scene);
	pixItem3->setScene(*scene);
	pixItem4->setScene(*scene);
	pixItem5->setScene(*scene);

	this->setInteractive(true);
	this->setMouseTracking(true);

	camItem->addToGroup(textItem1); // Index 0 == front
	camItem->addToGroup(pixItem1);  // Index 1
	camItem->addToGroup(textItem2); // Index 2
	camItem->addToGroup(pixItem2);  // Index 3
	camItem->addToGroup(textItem3); // Index 4
	camItem->addToGroup(pixItem3);  // Index 5
	camItem->addToGroup(textItem4); // Index 6
	camItem->addToGroup(pixItem4);  // Index 7
	camItem->addToGroup(textItem5); // Index 8
	camItem->addToGroup(pixItem5);  // Index 9 == back

	scene->addItem(camItem);

	qDebug() << "-" << this->objectName() << "Add Camera:"
			 << "Scene initialisiert.";
	camItem->show();

	// Camera management
	this->cam = cam;
	qDebug() << "-" << this->objectName() << "Add Camera:" << "Camera aktualisiert.";
	return true;
}

// wichtig!! Update-Routine
bool DetectObject::recordingUpdate()
{
	// Grab Images
	if (!cam->recordingUpdate_Grab())
	{
		qCritical() << "!!" << this->objectName() << "recordingUpdate:" << "Cannot grab from Camera";
		return false;
	}
	// Retrieve Images
	if (!cam->recordingUpdate_RetrieveImage())
	{
		qCritical() << "!!" << this->objectName() << "recordingUpdate:" << "Cannot retrieve Image from Camera";
		return false;
	}
	return true;
}

// wichtig
bool DetectObject::takePictureWithoutObject()
{
	if (!cam->getRecordedUndistortedImage(imageWithoutObject))
	{
		qCritical() << "!!" << this->objectName() << "takePictureWithoutObject:"
					<< "could not load (undistorted) image from Camera";
		return false;
	}
	cv::Mat image;
	cv::cvtColor(imageWithoutObject,image,CV_BGR2RGB);
	CameraPixmap * pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(1));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(image));
	return true;
}

// wichtig
bool DetectObject::takePictureWithObject()
{
	if (!cam->getRecordedUndistortedImage(imageWithObject))
	{
		qCritical() << "!!" << this->objectName() << "takePictureWithObject:"
					<< "could not load (undistorted) image from Camera";
		return false;
	}

	cv::Mat newimage;
	if (!cam->getRecordedImage(newimage))
	{
		qCritical() << "!!" << this->objectName() << "takePictureWithObject:"
					<< "could not load (undistorted) image from Camera";
		return false;
	}
	movingCamera.push_back(newimage);
	cv::Mat image;
	cv::cvtColor(imageWithObject,image,CV_BGR2RGB);
	CameraPixmap * pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(3));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(image));
	if (movingCameraCount <= 0) movingCameraCount++;
	return true;
}

// wichtig? -> weitere Bilder machen, die in einem Array gespeichert werden
bool DetectObject::takeNextMovingCameraPicture()
{
	assert(movingCameraCount > 0);
	cv::Mat newimage;
	if (!cam->getRecordedUndistortedImage(newimage))
	{
		qCritical() << "!!" << this->objectName() << "takePictureWithObject:"
					<< "could not load (undistorted) image from Camera";
		return false;
	}
	movingCamera.push_back(newimage);
	cv::Mat imageR;
	cv::cvtColor(newimage,imageR,CV_BGR2RGB);
	CameraPixmap * pItemL = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(1));
	CameraPixmap * pItemR = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(3));
	assert(pItemL);
	assert(pItemR);
	pItemL->setPixmap(pItemR);
	pItemR->setPixmap(Camera::cvtImageToPixmap(imageR));
	movingCameraCount++;
	return true;
}

// erster Test mit Histogrammen (Erzeugung), veraltet
void DetectObject::calibrateObjectsColor(bool showHist)
{
	// Histogramm and BackProjection
	cv::Mat hsv;
	cv::cvtColor(imageWithObject, hsv, CV_BGR2HSV);
	int hbins = 30, sbins = 32;
	int histSize[] = {hbins, sbins};
	float hranges[] = {0, 180};
	float sranges[] = {0, 256};
	const float * ranges[] = {hranges, sranges};
	int channels[] = {0, 1};
	/*
	int hbins = 30, sbins = 32;
	int histSize[] = {hbins, sbins};
	float hranges[] = {0, 180};
	float sranges[] = {0, 256};
	const float * ranges[] = {hranges, sranges};
	int channels[] = {0, 1};
	*/
	cv::calcHist(&hsv, 1, channels, cv::Mat(), objectHist,
				 2, histSize, ranges, true, false);
	if (showHist)
	{
		double maxValue = 0;
		cv::minMaxLoc(objectHist, 0, &maxValue, 0, 0);
		int scale = 10;
		cv::Mat histImage = cv::Mat::zeros(sbins*hbins, hbins*10, CV_8UC3);
		for (int h = 0; h < hbins; h++)
		{
			for (int s = 0; s < sbins; s++)
			{
				float binValue = objectHist.at<float>(h, s);
				int intensity = cvRound(binValue * 255/maxValue);
				cv::rectangle(histImage, cv::Point(h*scale, s*scale),
							  cv::Point((h+1)*scale-1, (s+1)*scale-1),
							  cv::Scalar::all(intensity), CV_FILLED);
			}
		}
		CameraPixmap * pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(7));
		assert(pItem);
		pItem->setPixmap(Camera::cvtImageToPixmap(histImage), true);
	}
}


// erster Test mit Histogrammen (Auswertung), veraltet
bool DetectObject::trackObjectByHist()
{
	cv::Mat currentImg;
	if (!cam->getRecordedUndistortedImage(currentImg))
	{
		qCritical() << "!!" << this->objectName() << "takePictureWithObject:"
					<< "could not load (undistorted) image from Camera";
		return false;
	}
	cv::Mat hsv, objectProbImage;
	cv::cvtColor(currentImg, hsv, CV_BGR2HSV);
	int hbins = 30, sbins = 32;
	int histSize[] = {hbins, sbins};
	float hranges[] = {0, 180};
	float sranges[] = {0, 256};
	const float * ranges[] = {hranges, sranges};
	int channels[] = {0, 1};
	cv::calcBackProject(&hsv, 1, channels, objectHist, objectProbImage,
						ranges, 1, true);
	qDebug() << "PRobImage berechet:" << objectProbImage.channels() << objectProbImage.type();
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(objectProbImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	qDebug() << "contours berechet:" << contours.size();
	cv::cvtColor(objectProbImage, objectProbImage, CV_GRAY2RGB);
	/*QList<double> areas;
	double min = -1;
	double max = 0;
	double mean = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		std::vector<cv::Point> contour = contours[i];
		double area = cv::contourArea(contour, false);
		areas.push_back(area);
		if (min < 0 || area < min) min = area;
		if (area > max) max = area;
		mean += area;
	}
	mean /= contours.size();

	qDebug() << "Contours: min,max,mean" << min << max << mean;

	for (int i = 0; i < contours.size(); i++)
	{
		std::vector<cv::Point> contour;
		double area = contourArea(contour, false);
		if (area >= mean)
			drawContours(objectProbImage, contours, i,
						 cv::Scalar(rand()%255, rand()%255, rand()%255));
	}
*/

	CameraPixmap * pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(7));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(objectProbImage), true);
	return true;
}

// wichtig!!
bool DetectObject::analyseColorSpace()
{
	cv::Mat currentImg, hsv, h;
	if (!cam->getRecordedUndistortedImage(currentImg))
	{
		qCritical() << "!!" << this->objectName() << "takePictureWithObject:"
					<< "could not load (undistorted) image from Camera";
		return false;
	}
	cv::GaussianBlur(currentImg, currentImg, cv::Size(7,7), 1.5);

	cv::Mat colorReduced;
	doColorReduce(
				currentImg, // input
				colorReduced, // output
				24 // div
				);

	cv::Mat objectBP;
	doHueBackProjection(
				colorReduced, // reduced color image
				objectBP // backPorjection with Treshold
				);

	cv::Mat watershed, segmentation;
	doWatershed(
				currentImg, // color image
				objectBP, // Histogramm-BackProjection
				watershed, // 8UC1
				segmentation // 8UC1
				);

	CameraPixmap * pItem1 = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(1));
	assert(pItem1);
	cv::Mat objectBP_RGB;
	cv::cvtColor(objectBP, objectBP_RGB, CV_GRAY2RGB);
	pItem1->setPixmap(Camera::cvtImageToPixmap(objectBP_RGB), true);

	CameraPixmap * pItem2 = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(3));
	assert(pItem2);
	cv::Mat watershed_RGB;
	cv::cvtColor(watershed, watershed_RGB, CV_GRAY2RGB);
	pItem2->setPixmap(Camera::cvtImageToPixmap(watershed_RGB), true);

	CameraPixmap * pItem3 = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(5));
	assert(pItem3);
	cv::cvtColor(segmentation, segmentation, CV_GRAY2RGB);
	pItem3->setPixmap(Camera::cvtImageToPixmap(segmentation), true);

	// watershed: objekt bleibt gleich, wand verändert sich immer mal wieder
	// durchschnitt an pixeln ermitteln und dann thresholden???

	// Nachfilterung
	//cv::GaussianBlur(objectBP, objectBP, cv::Size(3,3), 1.0);

	//#########################################################################
	//#########################################################################
	//cv::threshold(objectBP, objectBP, 100, 255, cv::THRESH_BINARY);

	/*
	// Finde Konturen
	std::vector<std::vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	//findContour_compareTwoImages(objectBP, contour, 0);
	cv::findContours(objectBP, contours, hierarchy,
					 CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	qDebug() << "-" << this->objectName() << "compare Pictures:"
			 << "anzahl Contours:" << contours.size();
	if (contours.size() <= 0)
	{
		qWarning() << "*" << this->objectName() << "compare Pictures:"
				   << "keine Konturen gefunden!";
		return false;
	}
	*/

	cv::cvtColor(objectBP, currentImg, CV_GRAY2RGB);

	/*
	qDebug() << "Durchlaufe Hierarchy...";
 // hierarchy aufteilung: (kein * == -1)
 // next index: hierarchy[x][0]
 // prev index: hierarchy[x][1]
 // child index: hierarchy[x][2]
 // parent index: hierarchy[x][3]
 // Beispiel: CV_RETR_CCOMP
 //		index 0 hat kein prev, kein child und kein parent, next=1
 //		index 1 hat prev=0, kein child, kein parent
 //		index 28 hat prev=27, next=30, child=29, kein parent
 //		index 29 hat kein next!, kein prev!, kein child, parent=28
 //		index 104 hat kein next!, prev=103, child=105, kein parent (und ist damit letzte top-Kontur)
 //		index 105 hat next=106, kein prev, kein child, parent=104
 //		index 106 hat next=107, prev=105, kein child, parent=104
 //	...	index 394 hat kein next, prev=393, kein child, parent=104 ENDE
	 for (int i = 0; i < hierarchy.size(); i++)
	 {
		 Vec4i h = hierarchy[i];
		 qDebug() << "Index" << i << "mit"
				  << h[0] << h[1] << h[2] << h[3];
	 }
	int objectIndex;
	unsigned int maxPoints = 0;
	for (int top = 0; top >= 0; top = hierarchy[top][0])
	{
		// top level: durchlaufe top->next
		qDebug() << "top:" << top;
		std::vector<cv::Point> contour = contours[top];
		// farbige Konturen im grauen Bild einzeichnen
		cv::drawContours(currentImg, contours, top,
						 cv::Scalar(0,255,255), 1, 8, hierarchy);
		int cchild = hierarchy[top][3];
		qDebug() << "first child:" << ((cchild<0)?-1:cchild);
		qDebug() << "closed arcLength (double):" << arcLength(contour, true);
		qDebug() << "oriented area (double):" << contourArea(contour, true);
		qDebug() << "is convex? (bool):" << isContourConvex(contour);

		assert(hierarchy[top][3] == -1);
		cv::Scalar color( 255, 0, 0 );
		// ist bei child ein eintrag?
		for (; cchild >= 0; cchild = hierarchy[cchild][0])
		{
			// second level: durchlaufe child->next
			// hier ist die gesuchte Kontur des SchwarmElements
			cv::RotatedRect rrect = fitEllipse(contours[cchild]);
			double ldiff = std::abs(rrect.size.height - rrect.size.width);
			qDebug() << "Index" << cchild << "ist Child mit diff:" << ldiff;
			if (ldiff < 20) qDebug() << "kleiner 20!";
			// weitere childs gibt es nicht!
			// wenn kein next mehr vorhanden, dann wird abgebrochen und bei top->next weitergemacht
		}
	}


	for (uint i = 0; i < contours.size(); i++)
	{
		std::vector<cv::Point> contour = contours[i];

		bool convex = isContourConvex(contour);
		double arcLClosed = arcLength(contour, true);
		double area = contourArea(contour, true);

		if (convex && arcLClosed > 0 && area != 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(255,0,0), 1, 8, cv::noArray());
		}
		else if (convex && arcLClosed > 0 && area == 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(0,0,255), 1, 8, cv::noArray());
		}
		else if (convex && arcLClosed <= 0 && area != 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(255,255,255), 1, 8, cv::noArray());
		}
		else if (convex && arcLClosed <= 0 && area == 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(100,0,0), 1, 8, cv::noArray());
		}
		else if (!convex && arcLClosed > 0 && area != 0 && arcLClosed < 5000)
		{
			qDebug() << "index:" << i << "mit next|prev|child|parent:"
					 << hierarchy[i][0] << hierarchy[i][1] << hierarchy[i][2] << hierarchy[i][3];

			// Hier ist das Objekt nahezu immer zu finden!
			qDebug() << "closed arcLength (double):" << arcLClosed;
			qDebug() << "oriented area (double):" << area;

//			if (70 < arcLClosed && arcLClosed <= 2000)
//			{
//				// Objekt meistens in diesem Bereich bei einem Abstand von ca. 1-3 Meter
//				// verkleinern der unteren Grenze vermehrt andere Konturen, vergrößern führt zur verkleinerung des maximalen abstands
//				cv::drawContours(currentImg, contours, i,
//							 cv::Scalar(255,rand()%100,0), 1, 8, cv::noArray());
//				// Prüfe Punkte der Kontur
//				cv::Point meanPoint = contour[0];
//				for (int j = 1; j < contour.size(); j++)
//				{
//					meanPoint += contour[i];
//				}
//				meanPoint.x /= contour.size();
//				meanPoint.y /= contour.size();
//				qDebug() << "MeanPoint:" << meanPoint.x << meanPoint.y;
//			}
//			else if (2000 < arcLClosed)
//			{
//				// Bei nahem ist das Objekt in diesem Bereich (< 1 Meter)
//				cv::drawContours(currentImg, contours, i,
//							 cv::Scalar(0,rand()%100,rand()%255), 1, 8, cv::noArray());
//			}


			//  Camshift
			cv::Rect rect = cv::boundingRect(contour);
			if (rect.x < 0 || rect.y < 0 ||
					(rect.x + rect.width) > objectBP.cols ||
					(rect.y + rect.height) > objectBP.rows)
				continue;
			RotatedRect rrect = cv::CamShift(
						objectBP,
						rect,
						cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 10, 1 )
						);
			qDebug() << "rotated rect:" << "center:" << rrect.center.x << rrect.center.y
					 << "angle:" << rrect.angle;

			//cv::rectangle(currentImg, rect, cv::Scalar(255,rand()%255,0), 2, 8);
		}
		else if (!convex && arcLClosed > 0 && area == 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(255,0,0), 1, 8, cv::noArray());
		}
		else if (!convex && arcLClosed <= 0 && area != 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(100,0,255), 1, 8, cv::noArray());
		}
		else if (!convex && arcLClosed <= 0 && area == 0)
		{
			//cv::drawContours(currentImg, contours, i,
			//				 cv::Scalar(255,0,100), 1, 8, cv::noArray());
		}
	}
	*/

	//#########################################################################




	CameraPixmap * pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(7));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(currentImg), true);
	return true;
}

bool DetectObject::calibrateObjectsDescriptors(uint angleDegree)
{
	if (imageWithObject.empty())
	{
		qCritical() << "!!" << this->objectName() << "calibrateObjectsDescriptors:"
				 << (imageWithObject.empty()?"Bild mit Objekt fehlt.":"");
		return false;
	}

	angleDegree = angleDegree % 359;

	RobustMatcher rmatcher;
	rmatcher.setConfidenceLevel(0.98); // default: 0.99
	rmatcher.setMinDistanceToEpipolar(1.0); // default: 3.0
	rmatcher.setRatio(0.65f); // default
	cv::Ptr<cv::FeatureDetector> pfd = new cv::SurfFeatureDetector(10);
	rmatcher.setFeatureDetector(pfd);
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	rmatcher.searchDescriptors(imageWithObject, keypoints, descriptors);



	cv::FileStorage file;
	QString filename = QString("%1/%2").arg(
				QString("../SchwarmElement"), // PATH
				QString("SchwarmElement-%1.desc").arg(angleDegree) // filename + extension
				);
	file.open(filename.toStdString().c_str(), cv::FileStorage::APPEND);
	if (!file.isOpened())
	{
		qCritical() << "!!" << this->objectName() << "calibrateObjectsDescriptors:"
				 << "Konnte keine SchwarmElement-Descriptoren speichern.";
		return false;
	}

	//file << "angle" << angleDegree;
	//file << "keypoints"



	return true;
}

// veraltet, findet Ecken. Diese werden dann getresholded
void DetectObject::findCorners(
	const cv::Mat & binaryImage, std::vector<cv::Point> & corners, double strength)
{
	cv::Mat cornerImage;
	cv::cornerHarris(binaryImage, cornerImage,
					 3, // neighborhood size
					 3,  // aperture size
					 0.01); // harris parameter

	cv::threshold(cornerImage, cornerImage,
				  strength, // threshold: alle Ecken werden angezeigt
				  255,	  // maxVal: in weiß (gray = CV_8UC1)
				  cv::THRESH_BINARY); // maxVal if Pixel > threshold; else 0

	for (int y = 0; y < cornerImage.cols; y++)
	{
		for (int x = 0; x < cornerImage.rows; x++)
		{
			if (cornerImage.at<double>(x,y))
				corners.push_back(cv::Point(x,y));
		}
	}
}

// veraltet, Input ist ein Differenz-Image: absdiff(img1, img2, imageDiff) oder imageDiff = img2-img1
// -> Ansatz: Vorher/Nachher-Vergleich, manchmal rauschstark
bool DetectObject::findContour_compareTwoImages(
	cv::Mat & imageDiff, std::vector<cv::Point> & objectContours,
	double strength)
{
	//qWarning() << "*" << this->objectName() << "compare Pictures:"
	//		   << "Verfaelschtes Ergebnis, wenn die Kamera zwischen den Bildern bewegt wurde!";

	// Konturen im binären Bild finden
	std::vector<std::vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	cv::findContours(imageDiff, contours, hierarchy,
					 CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	qDebug() << "-" << this->objectName() << "compare Pictures:"
			 << "anzahl Contours:" << contours.size();
	if (contours.size() <= 0)
	{
		qWarning() << "*" << this->objectName() << "compare Pictures:"
				   << "keine Konturen gefunden!";
		return false;
	}

	// In RGB-Space zurücktransformieren,
	// um später farbige Konturen im grauen Bild einzuzeichnen
	cv::cvtColor(imageDiff,imageDiff,CV_GRAY2RGB);

	// Suche die längste Kontur -> Kontur des Objekts
	int objectIndex;
	unsigned int maxPoints = 0;
	for (int i=0; i >= 0; i = hierarchy[i][0])
	{
		std::vector<cv::Point> curr = contours[i];
		qDebug() << "-" << this->objectName() << "compare Pictures:"
				 << "contour" << i << "anzahl Points:" << curr.size();
		if (curr.size() > maxPoints)
		{
			maxPoints = curr.size();
			objectIndex = i;
		}
		// farbige Konturen im grauen Bild einzeichnen (bis auf Rot)
		cv::Scalar color( rand()&128, rand()&255, rand()&255 );
		cv::drawContours(imageDiff, contours, i,
						 color, CV_FILLED, 8, hierarchy);
	}
	qDebug() << "-" << this->objectName() << "compare Pictures:"
			 << "contour" << objectIndex << "gefunden mit" << maxPoints << "Punkten";

	// Kontur des Objekts
	objectContours = contours[objectIndex];

	// Binary Image
	//cv::Mat maskContourInImage(imageAfter.size(), CV_8UC1, cv::Scalar(0));
	//cv::drawContours(maskContourInImage, contours, objectIndex,
	//				 cv::Scalar(255), CV_FILLED, 8, hierarchy);

	// Object Frame and Binary Frame (SubImage)
	//cv::Rect objectBorder = cv::boundingRect(cv::Mat(contours[objectIndex]));
	//cv::Mat maskContourInObjectFrame = maskContourInImage(objectBorder);
	//cv::Mat objectTempFrame = imageAfter(objectBorder);
	//objectFrame = cv::Mat(objectBorder.size(), CV_8UC3, cv::Scalar(0,0,0));
	//objectTempFrame.copyTo(objectFrame, maskContourInObjectFrame);
	return true;
}

// wichtige Funktion!!!
void DetectObject::calibrateZ0Plane(
	const std::vector<cv::Point2f> & fourPixelPoints,
	const std::vector<cv::Point2f> & fourObjectPoints
	)
{
	assert(fourPixelPoints.size() == fourObjectPoints.size());
	assert(fourPixelPoints.size() == 4);
	assert(fourObjectPoints[0] == cv::Point2f(0,0));

	Z0Plane_projection = cv::getPerspectiveTransform(fourPixelPoints, fourObjectPoints);
	Z0Plane_backprojection = cv::getPerspectiveTransform(fourObjectPoints, fourPixelPoints);
	assert(Z0Plane_projection.size() == cv::Size(3,3));
	assert(Z0Plane_backprojection.size() == cv::Size(3,3));
	Z0Plane_fourPixelPoints = fourPixelPoints;
	Z0Plane_fourObjectPoints = fourObjectPoints;



	// Überprüfe Transformation
	cv::Mat testImage = imageWithoutObject.clone();
	cv::Mat imageBirdsView;

	for (uint i = 1; i < fourPixelPoints.size(); i++)
	{
		cv::Point2f begin = fourPixelPoints[i-1];
		cv::Point2f end = fourPixelPoints[i];
		cv::Point2f mid = end - begin;
		cv::circle(testImage, mid, 3, cv::Scalar(255,0,0));
	}

	cv::warpPerspective(testImage, imageBirdsView, Z0Plane_projection, testImage.size());

	for (uint i = 1; i < fourPixelPoints.size(); i++)
	{
		cv::Point2f begin = fourObjectPoints[i-1];
		cv::Point2f end = fourObjectPoints[i];
		cv::Point2f mid = end - begin;
		cv::circle(imageBirdsView, mid, 3, cv::Scalar(0,255,0));
	}

	cv::cvtColor(testImage, testImage, CV_BGR2RGB);
	cv::cvtColor(imageBirdsView, imageBirdsView, CV_BGR2RGB);

	// Interface
	CameraPixmap * pItem;
	assert(camItem->childItems().size() >= 8);
	// Zeichne Polygone (Contours) in Background
	// erzeuge QGraphicPolygonItem's aus den Konturen
	pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(7));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(testImage));

	pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(9));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(imageBirdsView));
}


// erste Version der Kontur-Erkennung, veraltet..
bool DetectObject::detectSchwarmElementContour()
{
	if (imageWithoutObject.empty() || imageWithObject.empty())
	{
		qCritical() << "!!" << this->objectName() << "compare Pictures:"
				 << (imageWithoutObject.empty()?"Bild ohne Objekt fehlt.":"")
				 << (imageWithObject.empty()?"Bild mit Objekt fehlt.":"");
		return false;
	}

	// absolutes Differenz-Bild generieren
	cv::Mat imageDiff;
	cv::absdiff(imageWithObject, imageWithoutObject, imageDiff);

	// Binäres Image erstellen mittels Gray-Image und Threshold
	cv::cvtColor(imageDiff,imageDiff,CV_RGB2GRAY);
	cv::threshold(imageDiff, imageDiff, 100, 255, cv::THRESH_BINARY);

	std::vector<cv::Point> objectContours;
	bool succ = findContour_compareTwoImages(
				imageDiff, objectContours, 100);

	// Interface
	CameraPixmap * pItem;
	assert(camItem->childItems().size() >= 8);
	// Zeichne Difference
	pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(5));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(imageDiff));
	// Zeichne Polygone (Contours) in Background
	// erzeuge QGraphicPolygonItem's aus den Konturen
	//pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(7));
	//assert(pItem);
	//pItem->setPixmap(Camera::cvtImageToPixmap(objectFrame));
	return succ;
}

// gedebuged am Di, 20.09.11/21.09.11
// funktioniert/getestet, aber doch keine Verwendung dafür.. schade..
// Auswertung der 3D-Koordinaten ist zu aufwendig..
bool DetectObject::detect3DSchwarmElement(const cv::Point2f & nullPoint)
{
	if (movingCameraCount < 2)
	{
		qCritical() << "!!" << this->objectName() << "compareTwoMovingCameraPictures:"
					<< "Es werden mindestens 2 Bilder benötigt.";
		return false;
	}
	if (movingCameraCount == 2)
	{
		qDebug() << "-" << this->objectName() << "compareTwoMovingCameraPictures:"
				 << "Erster Bildervergleich wird gestartet..";
	}
	if (movingCameraCount == 3)
	{
		qDebug() << "-" << this->objectName() << "compareTwoMovingCameraPictures:"
				 << "Zweiter Bildervergleich wird gestartet..";
	}

	cv::Mat imageLeft  = movingCamera[movingCameraCount-1];
	cv::Mat imageRight = movingCamera[movingCameraCount-2];
	if (imageLeft.empty() || imageRight.empty())
	{
		qCritical() << "!!" << this->objectName() << "compare Pictures:"
				 << (imageLeft.empty()?"Linkes Bild fehlt.":"")
				 << (imageRight.empty()?"Rechtes Bild fehlt.":"");
		return false;
	}

	bool succ = object->reconstruct(imageLeft, imageRight, cam, nullPoint);
	qDebug() << "-" << this->objectName() << "compareTwoMovingCameraPictures:"
			 << "reconstruct" << movingCameraCount-1 << "liefert:" << succ;

	/*
	// Interface
	CameraPixmap * pItem;
	assert(camItem->childItems().size() >= 8);
	// Zeichne Difference
	pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(5));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(imageDiff));
	// Zeichne Polygone (Contours) in Background
	// erzeuge QGraphicPolygonItem's aus den Konturen
	pItem = qgraphicsitem_cast<CameraPixmap *>(camItem->childItems().at(7));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(objectFrame));
	*/
	return true;
}

// veraltet
void DetectObject::setDetectObjectFunction(enum DO_OBJECTDETECTFUNCTION mode)
{
	detectObjectFunktion = mode;
}

// #############################################################################
// VERALTET
// #############################################################################
/*
bool DetectObject::detectSchwarmElement(const cv::Mat & in, cv::Mat & out)
{
	cv::cvtColor(in,out,CV_BGR2RGB);
	return true;
}

bool DetectObject::getCurrentImageAndDetectObject()
{
	cv::Mat image;
	if (!cam->getRecordedUndistortedImage(image))
	{
		qCritical() << "!!" << this->objectName() << "recordingUpdate:" << "could not load (undistorted) image from Camera";
		return false;
	}

	// Objekterkennung: Input: CV_BGR, Output: CV_RGB!
	bool succObjectDetection = detectObject(image, image);

	// Objekt ausgeben
	CameraPixmap * pItem = (CameraPixmap *) camItem->childItems().back();
	pItem->setPixmap(Camera::cvtImageToPixmap(image));
	return succObjectDetection;
}

bool DetectObject::detectObject(const cv::Mat & in, cv::Mat & out)
{
	switch (detectObjectFunktion)
	{
	case DO_LED: return detectLED(in,out);
	case DO_SCHWARMELEMENT: return detectSchwarmElement(in,out);
	default: cv::cvtColor(in,out,CV_BGR2RGB); return true; // DO_NOOBJECT
	}
}

inline void DetectObject::pixelIsInRange(cv::Mat & i, int r, int c, int ch,
										 double low, double high)
{
	if (low > high)
	{
		// unerwünschter Bereich: mittig
		if (high < pixel(i,r,c,ch) && pixel(i,r,c,ch) < low)
			pixel(i,r,c,ch) = 0;
	}
	else // low <= high
	{
		// unerwünschter Bereich: oberer und unterer Rand
		if (pixel(i,r,c,ch) < low || high < pixel(i,r,c,ch))
			pixel(i,r,c,ch) = 0;
	}
}

bool DetectObject::detectLED(const cv::Mat &, cv::Mat & )
{
	return true;
}

*/



//###########################

void DetectObject::doWatershed(
	const cv::Mat & watershedInputImage, // color image
	const cv::Mat & binaryImg, // Histogramm-BackProjection
	cv::Mat & watershedBinaryImg, // 8UC1
	cv::Mat & segmentationGrayImg // 8UC1
	)
{
	// Watershed
	cv::Mat fg, bg;
	cv::erode(binaryImg, fg, cv::Mat(), cv::Point(-1,-1), 20); // white = fg
	cv::dilate(binaryImg, bg, cv::Mat(), cv::Point(-1,-1), 1); // 10-2 war gut für Objekt
	cv::threshold(bg, bg, 1, 128, cv::THRESH_BINARY_INV); // black(0) = bg

	cv::Mat markers;
	cv::Mat sum(binaryImg.size(), CV_8UC1, cv::Scalar(0)); // black Image
	sum = fg + bg;
	sum.convertTo(markers, CV_32SC1);

	cv::watershed(watershedInputImage, markers);
	markers.convertTo(segmentationGrayImg, CV_8UC1);
	markers.convertTo(watershedBinaryImg, CV_8UC1, 255, 255);
}

void DetectObject::doHueBackProjection(
	const cv::Mat & colorImage, // reduced color image
	cv::Mat & backProjectionBinaryImg
	)
{
	// HSV aufsplitten
	cv::Mat hsv;
	cv::cvtColor(colorImage, hsv, CV_BGR2HSV);
	std::vector<cv::Mat> hsvs;
	cv::split(hsv,hsvs);

	// Histogramm erstellen (mask wird nicht gebraucht) und normalisieren
	int hbins = 5;
	int histSize[] = {hbins};
	float hranges[] = {0, 180};
	const float * ranges[] = {hranges};
	int channels[] = {0};
	cv::calcHist(&hsv, 1, channels, cv::Mat(), objectHist,
				 1, histSize, ranges, true, false);
	cv::normalize(objectHist, objectHist, 1.0);

	// Backprojektion
	cv::calcBackProject(&hsv, 1, channels,
						objectHist, backProjectionBinaryImg,
						ranges, 255.0, true);

	// Threshold im unteren Bereich
	cv::threshold(backProjectionBinaryImg, backProjectionBinaryImg,
				  100, 255, cv::THRESH_BINARY);
}

void DetectObject::doColorReduce(
	const cv::Mat & colorImage,
	cv::Mat & colorReducedImage,
	int div
	)
{
	assert(colorImage.channels() >= 3);
	colorReducedImage = colorImage.clone();

	// nach dem Vorbild aus OpenCV 2: Computer Vision
	// Application Programming Cookbook
	// Robert Laganiere
	for (int i = 0; i < colorReducedImage.rows; i++)
	{
		for (int j = 0; j < colorReducedImage.cols; j++)
		{
			colorReducedImage.at<cv::Vec3b>(i,j)[0] = // Blue
					colorReducedImage.at<cv::Vec3b>(i,j)[0] / div * div + div/2;
			colorReducedImage.at<cv::Vec3b>(i,j)[1] = // Green
					colorReducedImage.at<cv::Vec3b>(i,j)[1] / div * div + div/2;
			colorReducedImage.at<cv::Vec3b>(i,j)[2] = // Red
					colorReducedImage.at<cv::Vec3b>(i,j)[2] / div * div + div/2;
		}
	}
}
