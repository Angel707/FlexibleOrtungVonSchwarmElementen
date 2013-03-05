#ifndef SCHWARMELEMENT_H
#define SCHWARMELEMENT_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include "Camera.h"
#include "robustmatcher.h"
#include "cvlibwrapper.h"

class SchwarmElement
{
public:
	SchwarmElement();
	~SchwarmElement(){}

	bool reconstruct(
		const cv::Mat & leftImage, const cv::Mat & rightImage,
		Camera * cam, const cv::Point2f & refPoint);

signals:

public slots:

private:
	Camera * cam;
	// Korrespondierende Eigenschaften (Index zusammengehörig)
	std::vector<cv::Mat> objectFrames;
	std::vector<std::vector<cv::KeyPoint> > keypoints; // Keypoints: cv::SurfFeatureDetector
	std::vector<cv::Mat> descriptors;
	// Rekonstruktion
	std::vector<cv::Point3f> object3DPoints;
	unsigned int currentIndex;

	bool calcExtrinsicsFromEssential(
		const cv::Mat & essential, cv::Mat & R, cv::Mat & T,
		const cv::Mat & K_Left, const cv::Mat & K_Right,
		const cv::Point2f & matchedPoint_Left, const cv::Point2f & matchedPoint_Right);

	double calculateDepth(const cv::Mat & R, const cv::Mat & T, const cv::Mat & Point3D);

	bool isInFrontOfCameras(
		const cv::Mat & R, const cv::Mat & T,
		const cv::Point2f & point_Left, const cv::Point2f & point_Right,
		const cv::Mat & K_Left, const cv::Mat & K_Right);
};

#endif // SCHWARMELEMENT_H
