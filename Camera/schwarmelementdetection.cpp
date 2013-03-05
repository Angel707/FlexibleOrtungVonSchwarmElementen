#include "schwarmelementdetection.h"

#include <QtDebug>
#include <assert.h>

SchwarmElementDetection::SchwarmElementDetection(QObject *parent) :
	QObject(parent)
{
	this->setObjectName("SchwarmElementDetector");
	analyseSuccessfull = true;
}

// SLOT
void SchwarmElementDetection::searchObjects(Camera * cam)
{
	assert(cam);
	if (analyseSuccessfull) return;
	if (!cam->isTransformationActivated())
	{
		qWarning() << ">(slot)>" << this->objectName() << "searchObjects:"
				 << "Transformation fuer" << cam->objectName() << "noch nicht gesetzt.";
		return;
	}
	qDebug() << ">(slot)>" << this->objectName() << "searchObjects:"
			 << "Starte Analyse fuer" << cam->objectName() << "...";
	cv::Mat imageToAnalyse, imageAnalysed;
	bool succ = cam->getRecordedUndistortedImage(imageToAnalyse);
	assert(succ);

	succ = analyseImage(imageToAnalyse, imageAnalysed);

	qDebug() << "-" << this->objectName() << "searchObjects:"
			 << "Analyse erfolgreich abgeschlossen:" << succ;

	if (succ) analyseSuccessfull = true;
}

// Input: Image, Output: Object(-Contour)s
bool SchwarmElementDetection::analyseImage(
	const cv::Mat & imageToAnalyse,
	cv::Mat & analysed)
{
	// Gaussian Blur (Weichzeichner): Vermindert verrauschte Pixel
	cv::Mat afterGauss;
	cv::GaussianBlur(imageToAnalyse, afterGauss, cv::Size(7,7), 1.5);
	qDebug() << "<(signal)<" << this->objectName() << "analyseImage:"
			 << "new Blur Image (BGR).";
	emit currentBlur(afterGauss);

	// Farbreduktion: Kleine Konturen werden gefiltert
	// (z.B. Drähte des SchwarmElements)
	cv::Mat afterColorReduced;
	doColorReduce(
				afterGauss, // input
				afterColorReduced, // output
				24 // div, Reduktionsfaktor: Je höher, desto weniger Farben
				);
	qDebug() << "<(signal)<" << this->objectName() << "analyseImage:"
			 << "new color reduced Image (BGR).";
	emit currentColorReduced(afterColorReduced);

	// Histogramm erstellen aus Hue und zurückprojezieren:
	// Beim Histogramm erstellen werden alle Hue Werte in x Bins aufgeteilt.
	// Es ist eine Art Farbreduktion.
	cv::Mat afterBackProjection;
	doHueBackProjection(
				afterColorReduced, // reduced color image
				afterBackProjection // backprojection with Treshold
				);
	qDebug() << "<(signal)<" << this->objectName() << "analyseImage:"
			 << "new Back Projection (Binary).";
	emit currentBackProjection(afterBackProjection);

	// Die HistogrammBackProjektion wird nach Vorder- und Hintergrundobjekten
	// gefiltert. Dann werden beide addiert, wodurch ein Marker-Bild entsteht.
	// Mit diesem wird das normale Bild analysiert.
	// Die Segmentationsbild enthält dunkle Objekte auf weißem Grund
	// Das Watershed-Bild enthält die Kuntur der Objekte auf weißem Grund.
	cv::Mat afterWatershed, afterSegmentation;
	doWatershed(
				afterGauss, // color image
				afterBackProjection, // Histogramm-BackProjection
				afterWatershed, // 8UC1
				afterSegmentation // 8UC1
				);
	qDebug() << "<(signal)<" << this->objectName() << "analyseImage:"
			 << "new Watershed (Binary) / Segmentation (Gray) Image.";
	emit currentWatershed(afterWatershed);
	emit currentSegmentation(afterSegmentation); // Vordergrund: grau

	// Ergebnis
	analysed = afterSegmentation;

	return getObjectPoints(afterSegmentation, afterBackProjection, 0);
}


void SchwarmElementDetection::doColorReduce(
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

void SchwarmElementDetection::doHueBackProjection(
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
	cv::Mat histogramm_hue;
	int hbins = 5;
	int histSize[] = {hbins};
	float hranges[] = {0, 180};
	const float * ranges[] = {hranges};
	int channels[] = {0};
	cv::calcHist(&hsv, 1, channels, cv::Mat(), histogramm_hue,
				 1, histSize, ranges, true, false);
	cv::normalize(histogramm_hue, histogramm_hue, 1.0);

	// Backprojektion
	cv::calcBackProject(&hsv, 1, channels,
						histogramm_hue, backProjectionBinaryImg,
						ranges, 255.0, true);

	// Threshold im unteren Bereich
	cv::threshold(backProjectionBinaryImg, backProjectionBinaryImg,
				  100, 255, cv::THRESH_BINARY);
}

void SchwarmElementDetection::doWatershed(
	const cv::Mat & watershedInputImage, // color image
	const cv::Mat & binaryImg, // Histogramm-BackProjection
	cv::Mat & watershedBinaryImg, // 8UC1
	cv::Mat & segmentationGrayImg // 8UC1
	)
{
	// Watershed
	cv::Mat fg, bg;
	cv::erode(binaryImg, fg, cv::Mat(), cv::Point(-1,-1), 10); // white = fg
	cv::dilate(binaryImg, bg, cv::Mat(), cv::Point(-1,-1), 2); // 10-2 war gut für Objekt
	cv::threshold(bg, bg, 1, 128, cv::THRESH_BINARY_INV); // black(0) = bg

	cv::Mat markers;
	cv::Mat sum(binaryImg.size(), CV_8UC1, cv::Scalar(0)); // black Image
	sum = fg + bg;
	sum.convertTo(markers, CV_32SC1);

	cv::watershed(watershedInputImage, markers);
	markers.convertTo(segmentationGrayImg, CV_8UC1);
	markers.convertTo(watershedBinaryImg, CV_8UC1, 255, 255);
}

bool SchwarmElementDetection::getObjectPoints(
	const cv::Mat & grayImage, const cv::Mat & grayImage2,
	Camera * cam)
{
	cv::Mat objects;
	cv::blur(grayImage, objects, cv::Size(3,3));
	cv::blur(objects, objects, cv::Size(11,11));
	//cv::addWeighted(grayImage, 1, objects, 0.5, 0, objects);

	//cv::threshold(objects, objects, 254, 255, cv::THRESH_BINARY);


	/*
	// Finde Konturen
	std::vector<std::vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	cv::findContours(objects, contours, hierarchy,
					 CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	if (contours.size() <= 0)
	{
		qWarning() << "*" << this->objectName() << "getObjectPoints:"
				   << "keine Konturen gefunden!";
		return false;
	}
	qDebug() << "-" << this->objectName() << "getObjectPoints:"
			 << "anzahl Contours:" << contours.size();

	// Grau-Bild nach RGB konvertieren, um farbige Konturen einzuzeichnen
	cv::cvtColor(objects, objects, CV_GRAY2BGR);

	// Untersuche jede Kontur:
	for (uint i = 0; i < contours.size(); i++)
	{
		std::vector<cv::Point> contour = contours[i];

		bool convex = isContourConvex(contour);
		double arcLClosed = arcLength(contour, true);
		double area = contourArea(contour, true);

		//cv::drawContours(objects, contours, i,
		//				 cv::Scalar(100,100,100), 1, 8, cv::noArray());

		if (convex && arcLClosed > 0 && area != 0)
		{
			//cv::drawContours(objects, contours, i,
			//				 cv::Scalar(255,0,0), 4, 8, cv::noArray());
		}
		else if (convex && arcLClosed > 0 && area == 0)
		{
			//cv::drawContours(objects, contours, i,
			//				 cv::Scalar(255,0,0), 2, 8, cv::noArray());
		}
		else if (convex && arcLClosed <= 0 && area != 0)
		{
			//cv::drawContours(objects, contours, i,
			//				 cv::Scalar(0,0,0), 4, 8, cv::noArray());
		}
		else if (convex && arcLClosed <= 0 && area == 0)
		{
			cv::drawContours(objects, contours, i,
							 cv::Scalar(255,0,0), 2, 8, cv::noArray());
		}
		else if (!convex && arcLClosed > 0 && area != 0)
		{
			//qDebug() << "index:" << i << "mit next|prev|child|parent:"
			//		 << hierarchy[i][0] << hierarchy[i][1] << hierarchy[i][2] << hierarchy[i][3];

			// Hier ist das Objekt nahezu immer zu finden!
			qDebug() << "closed arcLength (double):" << arcLClosed;
			qDebug() << "oriented area (double):" << area;

			cv::drawContours(objects, contours, i,
							 cv::Scalar(0,255,0), 2, 8, cv::noArray());

			/*
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
			//cv::rectangle(objects, rect, cv::Scalar(255,rand()%255,0), 2, 8);

		}
		else if (!convex && arcLClosed > 0 && area == 0)
		{
			//cv::drawContours(objects, contours, i,
			//				 cv::Scalar(255,0,0), 1, 8, cv::noArray());
		}
		else if (!convex && arcLClosed <= 0 && area != 0)
		{
			//cv::drawContours(objects, contours, i,
			//				 cv::Scalar(100,0,255), 1, 8, cv::noArray());
		}
		else if (!convex && arcLClosed <= 0 && area == 0)
		{
			//cv::drawContours(objects, contours, i,
			//				 cv::Scalar(255,0,100), 1, 8, cv::noArray());
		}
	}
*/
	qDebug() << "<(signal)<" << this->objectName() << "analyseImage:"
			 << "new Objects Image.";
	emit currentObjects(objects);
	return true;
}

/*
bool SchwarmElementDetection::getObjectPoints(
	const cv::Mat & grayImage,
	Camera * cam
	)
{
	cv::Mat objects = grayImage.clone();
	std::vector<cv::Vec3f> circles; // x,y,radius


	cv::cvtColor(objects, objects, CV_GRAY2BGR);

	// Für jedes gefundene Objekt: Untersuche reale Größe
	int size = circles.size();
	qDebug() << "-" << this->objectName() << "getObjectPoints:"
			 << "Anzahl Kreise gefunden:" << size;

	for (int i = 0; i < size; i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		// Transformation: Pixel -> World
		// um die reale Größe des Objekts zu ermitteln

		QString einheit;
		bool succ;
		cv::Point2f pointCenter, pointLeft, pointTop, pointRight, pointBottom;
		cv::Vec3f circle = circles.front();

		// Center
		succ = cam->transformPixelToWorld(
			QPointF(circle(0),circle(1)), pointCenter,
			einheit, einheit );
		if (!succ) return false;

		// Left
		succ = cam->transformPixelToWorld(
			QPointF(circle(0)-circle(2),circle(1)), pointLeft,
			einheit, einheit );
		if (!succ) return false;

		// Top
		succ = cam->transformPixelToWorld(
			QPointF(circle(0),circle(1)-circle(2)), pointTop,
			einheit, einheit );
		if (!succ) return false;

		// Right
		succ = cam->transformPixelToWorld(
			QPointF(circle(0)+circle(2),circle(1)), pointRight,
			einheit, einheit );
		if (!succ) return false;

		// Bottom
		succ = cam->transformPixelToWorld(
			QPointF(circle(0),circle(1)+circle(2)), pointBottom,
			einheit, einheit );
		if (!succ) return false;

		qDebug() << "-" << this->objectName() << "getObjectPoints:"
				 << "Mittelpunkt des Kreises:"
				 << circle(0) << circle(1) << "=>" << pointCenter.x << pointCenter.y << einheit;
		qDebug() << "-" << this->objectName() << "getObjectPoints:"
				 << "Radius:"
				 << circle(2) << "=>" << (pointRight.x - pointCenter.x) << einheit;

		// Analyse der Punkte
		if (SchwarmElementItem::isSchwarmElement(
					pointCenter, pointLeft, pointTop, pointRight, pointBottom
					))
		{
			// draw the circle center
			cv::circle( objects, center, 3, Scalar(0,255,0), -1, 8, 0 );
			// draw the circle outline
			cv::circle( objects, center, radius, Scalar(0,255,255), 3, 8, 0 );
			bool newitem = false;
			qDebug() << "<(signal)<" << this->objectName() << "getObjectPoints:"
					 << "(neues?) SchwarmElement gefunden.";
			emit newitem = newItemFound(QPoint(circle(0),circle(1)));
			if (newitem)
			{
				SchwarmElementItem * item =
						new SchwarmElementItem("Bitte Identifieren!");
				assert(item);
				qDebug() << "<(signal)<" << this->objectName() << "getObjectPoints:"
						 << "Neues SchwarmElement gefunden.";
				emit newItemFound(item);
			}
			else
			{
				qDebug() << "-" << this->objectName() << "getObjectPoints:"
						 << "Es war kein neues SchwarmElement!";
			}
		}
		else
		{
			// Kein SchwarmElement
			// draw the circle center
			cv::circle( objects, center, 3, Scalar(255,0,0), -1, 8, 0 );
			// draw the circle outline
			cv::circle( objects, center, radius, Scalar(255,0,255), 3, 8, 0 );
		}
		// weiter mit nächstem Objekt..
	}
	emit currentObjects(objects);
	return true;
}
*/

void SchwarmElementDetection::analyse()
{
	analyseSuccessfull = false;
}
