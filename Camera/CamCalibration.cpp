/*
 * CamCalibration.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: Angelos Drossos
 */

#include "CamCalibration.h"

#include <QtDebug>

#include <iostream>
#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <cstring>

/*
string type("Unknown");
switch (gray.type() % 7)
{
	case CV_8U: 	type = "8-bit Unsigned"; break;
	case CV_8S: 	type = "8-bit Signed"; break;
	case CV_16U: 	type = "16-bit Unsigned"; break;
	case CV_16S: 	type = "16-bit Signed"; break;
	case CV_32S: 	type = "32-bit Signed"; break;
	case CV_32F: 	type = "32-bit FloatingPoints"; break;
	case CV_64F: 	type = "64-bit FloatingPoints"; break;
}
	printf("Processing a %dx%d image with %d channels and type '%s'\n",
			gray.cols, gray.rows, gray.channels(), type.c_str());
 */

CamCalibration::CamCalibration()
	: capture_speed(1000), image_count(0), chessSize(Size(0,0)){}

CamCalibration::CamCalibration(unsigned int rows, unsigned int cols)
	: capture_speed(1000), image_count(0)
{
	init(rows, cols);
}

void CamCalibration::init(unsigned int rows, unsigned int cols)
{
	chessSize = Size(cols, rows);
	generateObjectPoints(chessSize);
}

CamCalibration::~CamCalibration(void){}

void CamCalibration::generateObjectPoints(Size chessSize)
{
	for (int row = 0; row < chessSize.width; row++)
	{
		for (int col = 0; col < chessSize.height; col++)
		{
			chessPoints.push_back(Point3f(col, row, 0.0f));
		}
	}
}

vector<Point2f> CamCalibration::generateObjectPoints2D(Size chessSize, double w_scale, double h_scale)
{
	vector<Point2f> objectChessPoints;
	for (int row = 0; row < chessSize.width; row++)
	{
		for (int col = 0; col < chessSize.height; col++)
		{
			objectChessPoints.push_back(Point2f(col*w_scale, row*h_scale));
		}
	}
	return objectChessPoints;
}

bool CamCalibration::testCalibrationImage
(const Mat& gray, bool draw, Mat * drawImage, vector<Point2f> & corners)
{
	if (gray.type() != CV_8U)
	{
		qDebug() << "Please use gray images (type 8-bit unsigned) to calibrate the camera!";
		return false;
	}

	int chess_flags = cv::CALIB_CB_FAST_CHECK;
	chess_flags += cv::CALIB_CB_ADAPTIVE_THRESH;
	chess_flags += cv::CALIB_CB_NORMALIZE_IMAGE;
	qDebug() << "chessBoard:" << chessSize.width << "x" << chessSize.height << "(WxH)";
	bool patternWasFound = cv::findChessboardCorners(gray, chessSize,
			corners, chess_flags); // close all Image-Windows!
	qDebug() << "Bild untersucht:"
			 << "found corners:" << corners.size() << "von" << chessSize.area();
	if ((!patternWasFound) && corners.size() < 0.9*chessSize.area())
	{
		if (corners.size() == 0) qDebug() << "Chessboard NICHT gefunden.";
		else qDebug() << "Zu wenig Ecken am Chessboard gefunden:" << corners.size();
		return false;
	}
	if ((int)corners.size() == chessSize.area()) qDebug() << "Chessboard komplett gefunden";
	// Versuche genauere Werte zu ermitteln
	cv::cornerSubPix(gray, corners, // Image -> Corners
			cv::Size(5,5), // winSize: (5,6)=(5*2+1)x(6*2+1)-Suchfenster, Außen-Radius des Kreises
			cv::Size(-1,-1), // zeroZone: (-1,-1)=kein Fenster, Innen-Radius des Kreises
			cv::TermCriteria(
					cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS,
					30, // max iterations
					0.1 // min accuracy
			)
	);
	if (draw)
	{
		if (drawImage == NULL)
		{
			qCritical() << "Invalid drawImage-Pointer!";
			return false;
		}
		if ((drawImage->cols != gray.cols) && (drawImage->rows != gray.rows))
		{
			qDebug() << "drawImage & gray unterschiedlich gross!";
			return false;
		}
		qDebug() << "draw chessboard corners";
		cv::drawChessboardCorners(*drawImage, chessSize, corners, patternWasFound);
	}
	return true;
}

bool CamCalibration::tryCalibrationImage
(const Mat& gray, bool draw, Mat * drawImage)
{
	vector<Point2f> corners;
	return testCalibrationImage(gray, draw, drawImage, corners);
}

bool CamCalibration::addCalibrationImage
(const Mat& gray, bool draw, Mat * drawImage)
{
	vector<Point2f> corners;
	if (!testCalibrationImage(gray, draw, drawImage, corners))
	{
		return false;
	}
	if ((int)corners.size() != chessSize.area())
	{
		// Fehler: Nicht alle Punkte gefunden
		return false;
	}

	// Alle Punkte gefunden
	// Punkte hinzufügen: imagePoints und objectPoints
	imagePoints.push_back(corners);
	objectPoints.push_back(chessPoints);
	image_count++;
	qDebug() << "Punkte wurden hinzugefuegt.";
	return true;
}

bool CamCalibration::calibrateCam(Mat& camMat, Mat& distCoeffs,
			vector<Mat>& rvecs, vector<Mat>& tvecs)
{
	if (image_count < 2)
	{
		qDebug() << "Zu wenig Bilder zum Calibrieren.";
		return false;
	}
	qDebug() << "Es wird mit" << image_count << "Bildern calibriert.";
	if (image_count < 4)
	{
		qDebug() << "Hinweis: Je mehr Bilder zur Verfuegung stehen, desto besser die Calibrierung.";
	}
	double ret = cv::calibrateCamera(
			objectPoints, 	// InputArrayOfArrays
			imagePoints,	// InputArrayOfArrays
			chessSize,		// Size of chessboard
			camMat,			// InputOutputMatrix
			distCoeffs,		// InputOutputArray
			rvecs,			// OutputArray
			tvecs,			// OutputArray
			0				// Flags
	);
	qDebug() << "calibrateCamera gab zurueck:" << ret;
	return true;
}

bool CamCalibration::findOneMoreChess(const Mat& gray, Mat& homography)
{
	vector<Point2f> corners;
	int chess_flags = cv::CALIB_CB_FAST_CHECK;
		chess_flags += cv::CALIB_CB_ADAPTIVE_THRESH;
		chess_flags += cv::CALIB_CB_NORMALIZE_IMAGE;
	bool patternWasFound = cv::findChessboardCorners(gray, chessSize,
				corners, chess_flags); // close all Image-Windows!
	cv::cornerSubPix(gray, corners, // Image -> Corners
			cv::Size(5,5), // winSize: (5,6)=(5*2+1)x(6*2+1)-Suchfenster, Außen-Radius des Kreises
			cv::Size(-1,-1), // zeroZone: (-1,-1)=kein Fenster, Innen-Radius des Kreises
			cv::TermCriteria(
					cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS,
					30, // max iterations
					0.1 // min accuracy
			)
	);
	cv::Mat grayDraw = gray.clone();
	cv::drawChessboardCorners(grayDraw, chessSize, corners, patternWasFound);
	cv::imshow("Chessboard-Analysis", grayDraw);
	if ((int)corners.size() == chessSize.area())
	{
		// Alle Punkte gefunden
		double w_scale = 10; // mm
		double h_scale = 10; // mm
		// Calc. Object Points
		vector<Point2f> objectCorners = generateObjectPoints2D(chessSize, w_scale, h_scale);
		homography = cv::findHomography(corners, objectCorners);
		return true;
	}
	return false;
}
