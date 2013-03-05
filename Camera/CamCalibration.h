/*
 * CamCalibration.h
 *
 *  Created on: Aug 12, 2011
 *      Author: Angelos Drossos
 */

#ifndef CAMCALIBRATION_H_
#define CAMCALIBRATION_H_

// OpenCV Libs
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class CamCalibration
{
public:
	CamCalibration();
	void init(unsigned int rows, unsigned int cols);
	CamCalibration(unsigned int number_of_rows, unsigned int number_of_cols);
	virtual ~CamCalibration(void);

	bool tryCalibrationImage(const Mat& image_8U, bool drawFoundPattern = false, Mat * drawImage = NULL);
	bool addCalibrationImage(const Mat& image_8U, bool drawFoundPattern = false, Mat * drawImage = NULL);
	bool calibrateCam(Mat& cameraMatrix, Mat& distributionCoefficients,
			vector<Mat>& rotationVector, vector<Mat>& translationVector);

	bool findOneMoreChess(const Mat& gray_undistorted, Mat& homography);

private:
	/// Pause in ms bis zur nächsten Bildauswertung:
	/// 0: keine Pause. Default ist 1 sec.
	unsigned int capture_speed;

	/// Anzahl von Images
	unsigned int image_count;

	/// chessSize (without border-points)
	Size chessSize;
	/// 3D Chess Points
	vector<Point3f> chessPoints;

	/// 3D Calibration Points
	vector<vector<Point3f> > objectPoints;
	/// 2D Calibration Points
	vector<vector<Point2f> > imagePoints;

	/// Generiert das Chessboard-Pattern in Weltkoordinaten
	void generateObjectPoints(cv::Size patternSize);

	bool testCalibrationImage
	(const Mat& gray, bool draw, Mat * drawImage, vector<Point2f> & corners);

	vector<Point2f> generateObjectPoints2D(Size chessSize, double width_mm, double height_mm);
};

#endif /* CAMCALIBRATION_H_ */
