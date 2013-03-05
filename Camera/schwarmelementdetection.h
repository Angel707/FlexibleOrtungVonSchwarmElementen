#ifndef SCHWARMELEMENTDETECTION_H
#define SCHWARMELEMENTDETECTION_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include "Camera.h"
#include "schwarmelementitem.h"

class SchwarmElementDetection : public QObject
{
	Q_OBJECT
public: //=====================================================================
	explicit SchwarmElementDetection(QObject *parent = 0);

signals: //====================================================================
	void newItemFound(SchwarmElementItem * item);
	bool newItemFound(const QPoint & itemPos);
	void currentBlur(cv::Mat imageBGR);
	void currentColorReduced(cv::Mat imageBGR);
	void currentBackProjection(cv::Mat imageGray);
	void currentWatershed(cv::Mat imageGray);
	void currentSegmentation(cv::Mat imageGray);
	void currentObjects(cv::Mat imageBGR);

public slots: //===============================================================
	//! Aktiviert die nächste Analyse
	void analyse();
	//! Sucht Objekte, wenn die Analyse (analyse()) aktiviert worden ist.
	//! Wenn Objekte gefunden wurden. Dann wird die Analyse deaktiviert,
	//! um Performance zu sparen.
	void searchObjects(Camera * cam);


private: //====================================================================
	bool analyseSuccessfull;
	// Funktionen
	bool analyseImage(
		const cv::Mat & imageToAnalyse,
		cv::Mat & analysed);

	void doColorReduce(
		const cv::Mat & colorImage,
		cv::Mat & colorReducedImage,
		int div
		);

	void doHueBackProjection(
		const cv::Mat & colorImage, // reduced color image
		cv::Mat & backProjectionBinaryImg
		);

	void doWatershed(
		const cv::Mat & watershedInputImage, // color image
		const cv::Mat & binaryImg, // Histogramm-BackProjection
		cv::Mat & watershedBinaryImg, // 8UC1
		cv::Mat & segmentationGrayImg // 8UC1
		);

	bool getObjectPoints(
		const cv::Mat & grayImage, const cv::Mat & grayImage2,
		Camera * cam
		);
};

#endif // SCHWARMELEMENTDETECTION_H
