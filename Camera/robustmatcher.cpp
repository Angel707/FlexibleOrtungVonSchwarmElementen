#include "robustmatcher.h"

#include <QtDebug>
#include <assert.h>

RobustMatcher::RobustMatcher() :
	ratio(0.65f), refineF(true), confidence(0.99), distance(3.0)
{
	// SURF is the default feature
	detector = new cv::SurfFeatureDetector();
	extractor = new cv::SurfDescriptorExtractor();
}

void RobustMatcher::setFeatureDetector(cv::Ptr<cv::FeatureDetector> & d)
{
	detector = d;
}

void RobustMatcher::setDescriptorExtractor(cv::Ptr<cv::DescriptorExtractor> & e)
{
	extractor = e;
}

void RobustMatcher::setConfidenceLevel(double newConfidence)
{
	confidence = newConfidence;
}

void RobustMatcher::setMinDistanceToEpipolar(double newDistance)
{
	distance = newDistance;
}

void RobustMatcher::setRatio(float newRatio)
{
	ratio = newRatio;
}

void RobustMatcher::setRefineF(bool refineF)
{
	this->refineF = refineF;
}

void RobustMatcher::searchDescriptors(
	const cv::Mat & image,
	std::vector<cv::KeyPoint> & keypoints,
	cv::Mat & descriptors)
{
	// 1a. Detection of the (SURF) features
	detector->detect(image, keypoints);

	qDebug() << "-" << "RobustMatcher" << "searchDescriptors:"
			 << "Features detected:"
			 << keypoints.size();

	// 1b. Extraction of the (SURF) features
	extractor->compute(image, keypoints, descriptors);

	qDebug() << "-" << "RobustMatcher" << "searchDescriptors:"
			 << "Extracted Descriptors:"
			 << "("<< descriptors.rows << "x" << descriptors.cols << ")";
}

void RobustMatcher::matchPrepare(const cv::Mat & image1, const cv::Mat & image2,
								 std::vector<cv::DMatch> & symMatches,
								 std::vector<cv::KeyPoint> & keypoints1,
								 std::vector<cv::KeyPoint> & keypoints2)
{
	// 1. feature detection and extraction
	cv::Mat descriptors1, descriptors2;
	searchDescriptors(image1, keypoints1, descriptors1);
	searchDescriptors(image2, keypoints2, descriptors2);

	// 2. Match the two image descriptors

	// Construction of the matcher
	cv::BruteForceMatcher<cv::L2<float> > matcher;

	// from image 1 to image 2
	// based on k nearst neighbours (with k=2)
	std::vector<std::vector<cv::DMatch> > matches1;
	matcher.knnMatch(descriptors1, descriptors2,
					 matches1, // vector of matches (up to 2 per entry)
					 2); // return 2 nearst neighbours

	// from image 2 to image 1
	// based on k nearst neighbours (with k=2)
	std::vector<std::vector<cv::DMatch> > matches2;
	matcher.knnMatch(descriptors2, descriptors1,
					 matches2, // vector of matches (up to 2 per entry)
					 2); // return 2 nearst neighbours

	qDebug() << "-" << "RobustMatcher" << "matchPrepare:"
			 << "Matches (BruteForceMatcher::knnMatch, Image 1->2 and 2->1):"
			 << matches1.size() << "and" << matches2.size();

	// 3. Remove matches for which NN is: ratio > threshold

	int removed;
	// clean image 1 -> image 2 matches
	removed = ratioTest(matches1);
	// clean image 2 -> image 1 matches
	removed = ratioTest(matches2);

	qDebug() << "-" << "RobustMatcher" << "matchPrepare:"
			 << "Matches (Filter ratioTest, Image 1->2 and 2->1):"
			 << matches1.size() << "and" << matches2.size();

	// 4. Remove non-symmetrical matches

	symmetryTest(matches1, matches2, symMatches);

	qDebug() << "-" << "RobustMatcher" << "matchPrepare:"
			 << "Matches (Filter symmetryTest):"
			 << symMatches.size();
}

cv::Mat RobustMatcher::match(const cv::Mat & image1, const cv::Mat & image2,
			  std::vector<cv::DMatch> & matches,
			  std::vector<cv::KeyPoint> & keypoints1,
			  std::vector<cv::KeyPoint> & keypoints2)
{
	std::vector<cv::DMatch> symMatches;
	matchPrepare(image1, image2, symMatches,
				 keypoints1, keypoints2);

	// 5. Validate matches using RANSAC

	std::vector<cv::Point2f> points1, points2;
	cv::Mat fundamental = ransacTest(symMatches, keypoints1, keypoints2,
									 matches, points1, points2);

	qDebug() << "-" << "RobustMatcher" << "match:"
			 << "Matches (Filter ransacTest):"
			 << matches.size();

	// Return the found fundamental matrix
	return fundamental;
}

cv::Mat RobustMatcher::match(const cv::Mat & image1, const cv::Mat & image2,
			  std::vector<cv::DMatch> & matches,
			  std::vector<cv::Point2f> & points1,
			  std::vector<cv::Point2f> & points2)
{
	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	std::vector<cv::DMatch> symMatches;
	matchPrepare(image1, image2, symMatches,
				 keypoints1, keypoints2);

	// 5. Validate matches using RANSAC

	cv::Mat fundamental = ransacTest(symMatches, keypoints1, keypoints2,
									 matches, points1, points2);

	qDebug() << "-" << "RobustMatcher" << "match:"
			 << "Matches (Filter ransacTest):"
			 << matches.size();

	// Return the found fundamental matrix
	return fundamental;
}

int RobustMatcher::ratioTest(std::vector<std::vector<cv::DMatch> > & matches)
{
	int removed = 0;
	// for all matches
	for (std::vector<std::vector<cv::DMatch> >::iterator mi = matches.begin();
		 mi != matches.end();
		 ++mi)
	{
		// if 2 NN has been identified
		if (mi->size() > 1)
		{
			// check distance ratio
			if ( (( (*mi)[0].distance )/( (*mi)[1].distance )) > ratio )
			{
				mi->clear(); // remove match
				removed++;
			}
		}
		else
		{
			// does not have 2 neighbours
			mi->clear(); // remove match
			removed++;
		}
	}
	return removed;
}

void RobustMatcher::symmetryTest(
	const std::vector<std::vector<cv::DMatch> > & matches1,
	const std::vector<std::vector<cv::DMatch> > & matches2,
	std::vector<cv::DMatch> & symMatches)
{
	// for all matches image 1 -> image 2
	for (std::vector<std::vector<cv::DMatch> >::const_iterator mi1 = matches1.begin();
		 mi1 != matches1.end();
		 ++mi1)
	{
		// ignore deleted matches
		if (mi1->size() < 2) continue;
		//for all matches image 2 -> image 1
		for (std::vector<std::vector<cv::DMatch> >::const_iterator mi2 = matches2.begin();
			 mi2 != matches2.end();
			 ++mi2)
		{
			// ignore deleted matches
			if (mi2->size() < 2) continue;
			// Match symmetry test -- begin
			if ( (*mi1)[0].queryIdx == (*mi2)[0].trainIdx
					&& (*mi2)[0].queryIdx == (*mi1)[0].trainIdx )
			{
				// add symmetrical match
				symMatches.push_back(
							cv::DMatch((*mi1)[0].queryIdx,
									   (*mi1)[0].trainIdx,
									   (*mi1)[0].distance)
							);
				break; // next match in image 1 -> image 2
			}
			// Match symmetry test -- end
		}
	}
}

cv::Mat RobustMatcher::ransacTest(
	const std::vector<cv::DMatch> & matches,
	const std::vector<cv::KeyPoint> & keypoints1,
	const std::vector<cv::KeyPoint> & keypoints2,
	std::vector<cv::DMatch> & ransacMatches,
	std::vector<cv::Point2f> & points1,
	std::vector<cv::Point2f> & points2)
{
	// convert keypoints int Point2f
	for (std::vector<cv::DMatch>::const_iterator mi = matches.begin();
		 mi != matches.end();
		 ++mi)
	{
		float x,y;
		// Get the position of left keypoints
		x = keypoints1[mi->queryIdx].pt.x;
		y = keypoints1[mi->queryIdx].pt.y;
		points1.push_back(cv::Point2f(x,y));
		// Get the position of right keypoints
		x = keypoints2[mi->queryIdx].pt.x;
		y = keypoints2[mi->queryIdx].pt.y;
		points2.push_back(cv::Point2f(x,y));
	}

	// Compute F (fundamental) matrix using RANSAC
	std::vector<uchar> inliers(points1.size(), 0);
	cv::Mat fundamental = cv::findFundamentalMat(
				cv::Mat(points1), cv::Mat(points2), // matching points
				inliers,		// match status (inlier or outlier)
				CV_FM_RANSAC,	// RANSAC method
				distance,		// distance to epipolar line
				confidence		// confidence probability
				);

	// extract the surviving (inliers) matches
	std::vector<uchar>::const_iterator ii = inliers.begin();
	std::vector<cv::DMatch>::const_iterator mi = matches.begin();
	// for all matches
	for ( ; ii != inliers.end(); ++ii, ++mi)
	{
		if (*ii)
		{
			// iterator is a valid match
			ransacMatches.push_back(*mi);
		}
	}

	// refineF = true: The F matrix will be recomputed with all accepted matches
	if (refineF)
	{
		// Convert keypoints into Point2f for final computation
		points1.clear();
		points2.clear();
		for (std::vector<cv::DMatch>::const_iterator mi = ransacMatches.begin();
			 mi != ransacMatches.end();
			 ++mi)
		{
			float x, y;
			// Get the position of left keypoints
			x = keypoints1[mi->queryIdx].pt.x;
			y = keypoints1[mi->queryIdx].pt.y;
			points1.push_back(cv::Point2f(x,y));
			// Get the position of right keypoints
			x = keypoints2[mi->queryIdx].pt.x;
			y = keypoints2[mi->queryIdx].pt.y;
			points2.push_back(cv::Point2f(x,y));
		}

		// Compute 8-point F from all accepted matches
		fundamental = cv::findFundamentalMat(
					cv::Mat(points1), cv::Mat(points2), // matches
					CV_FM_8POINT	// 8-point method
					);
	}

	return fundamental;
}

// =============================================================================
// STATIC
// =============================================================================

int RobustMatcher::getFundamental(
	const cv::Mat & image1, const cv::Mat & image2,
	cv::Mat & fundamental)
{
	// Prepare the matcher
	RobustMatcher rmatcher;
	rmatcher.setConfidenceLevel(0.98); // default: 0.99
	rmatcher.setMinDistanceToEpipolar(1.0); // default: 3.0
	rmatcher.setRatio(0.65f); // default
	rmatcher.setRefineF(true); // default
	cv::Ptr<cv::FeatureDetector> pfd = new cv::SurfFeatureDetector(10);
	rmatcher.setFeatureDetector(pfd);
	// Match the two images
	std::vector<cv::DMatch> matches;
	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	fundamental = rmatcher.match(image1, image2,
								 matches,
								 keypoints1, keypoints2);
	return 100; // 0: failed
}

bool RobustMatcher::getEssential(
	Camera * cam, const cv::Mat & fundamental, cv::Mat & essential)
{
	assert(cam);
	return cam->getEssentialMatrix(fundamental, essential);
}
