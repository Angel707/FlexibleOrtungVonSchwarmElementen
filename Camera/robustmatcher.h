#ifndef ROBUSTMATCHER_H
#define ROBUSTMATCHER_H

#include <opencv2/opencv.hpp>
#include "Camera.h"
#include "cvlibwrapper.h"

/*
 * Quelle: Buch
 * Titel: OpenCV 2: Computer Vision Application Programming Cookbook,
 * Author: Robert Langaniere
 * First Published: May 2011
 * Der Quellcode stammt größtenteils nicht von Angelos Drossos, sondern
 * aus o.g. Buch!
 *
 * Die Funktionen ransacTest() und match() wurden minimal abgewandelt.
 * Es werden nun die ermittelten 2D-Punkte ausgegeben.
 * Die erneute Berechnung der Fundamentalmatrix ist zu Debug-Zwecken
 * beibehalten, aber überflüssig geworden, da die Fundamentalmatrix F
 * in einer gesonderten StereoCalibration berechnet wird.
 *
 * Diese StereoCalibration wird außerhalb des Matchers stattfinden,
 * da die Aufgabe "Korrespondierende Punkte in beiden Bildern finden" erfüllt
 * ist. Vorteil dieser Variante ist das Berechnen / Überprüfen der intrinsischen
 * Kamera-Parameter sowie das Berechnen der Translation und Rotation zwischen
 * den beiden übergebenen Bilder.
 * Translation und Rotation werden dann zur Berechnung der 3D-Koordinaten
 * benötigt.
 */

class RobustMatcher
{
public:
	RobustMatcher();
	/// Set the feature detector
	void setFeatureDetector(cv::Ptr<cv::FeatureDetector> & detector);
	/// Set the descriptor extractor
	void setDescriptorExtractor(cv::Ptr<cv::DescriptorExtractor> & extractor);

	void setConfidenceLevel(double newConfidence);

	void setMinDistanceToEpipolar(double newDistance);

	void setRatio(float newRatio);

	void setRefineF(bool refineF);

	cv::Mat match(const cv::Mat & image1, const cv::Mat & image2,	// inputs
				  std::vector<cv::DMatch> & matches,				// outputs
				  std::vector<cv::KeyPoint> & keypoints1,
				  std::vector<cv::KeyPoint> & keypoints2);

	cv::Mat match(const cv::Mat & image1, const cv::Mat & image2,	// inputs
				  std::vector<cv::DMatch> & matches,				// outputs
				  std::vector<cv::Point2f> & points1,
				  std::vector<cv::Point2f> & points2);

	void searchDescriptors(
		const cv::Mat & image,
		std::vector<cv::KeyPoint> & keypoints,
		cv::Mat & descriptors);

	//! get fundamental matrix from two images
	static int getFundamental(const cv::Mat & image1, const cv::Mat & image2,
						cv::Mat & fundamental);

	//! get essential matrix from a calibrated camera and fundamental matrix.
	//! function returns false, if camera is not calibrated.
	static bool getEssential(
		Camera * cam, const cv::Mat & fundamental, cv::Mat & essential);

private:

	/// max ratio between 1st and 2nd NN
	float ratio;

	/// if true will refine the F (fundamental) matrix
	bool refineF;

	/// confidence level (probability)
	double confidence;

	/// minimal distance to epipolar
	double distance;

	/// pointer to the feature point detector object
	cv::Ptr<cv::FeatureDetector> detector;

	/// pointer to the feature descriptor extractor object
	cv::Ptr<cv::DescriptorExtractor> extractor;

	void matchPrepare(
		const cv::Mat & image1, const cv::Mat & image2,
		std::vector<cv::DMatch> & symMatches,
		std::vector<cv::KeyPoint> & keypoints1,
		std::vector<cv::KeyPoint> & keypoints2);

	int ratioTest(
		std::vector<std::vector<cv::DMatch> > & matches);

	void symmetryTest(
		const std::vector<std::vector<cv::DMatch> > & matches1,
		const std::vector<std::vector<cv::DMatch> > & matches2,
		std::vector<cv::DMatch> & inlierMatches);

	cv::Mat ransacTest(
		const std::vector<cv::DMatch> & matches,
		const std::vector<cv::KeyPoint> & keypoints1,
		const std::vector<cv::KeyPoint> & keypoints2,
		std::vector<cv::DMatch> & inlierMatches,
		std::vector<cv::Point2f> & points1,
		std::vector<cv::Point2f> & points2);

};

#endif // ROBUSTMATCHER_H
