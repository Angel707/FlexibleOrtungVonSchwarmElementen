#ifndef CVLIBWRAPPER_H
#define CVLIBWRAPPER_H

/*
 * Computer Vision Library Wrapper:
 *
 * Wrapper zwischen Qt und OpenCV 2. Er bietet auch Funktion,
 * um Matrizen und Vektoren verschiedener Art als String ausgeben zu können.
 * QString bietet dabei auch eine Funktion, um QString in "const char *" oder
 * "std::string" umzuformen.
 *
 */

#include <QString>
#include <opencv2/opencv.hpp>

namespace CVLibWrapper
{
	QString cvtMatrixToString(const cv::Mat & matrix, const char * description = "");
	QString cvtVectorToString(const std::vector<cv::Point2f> vector, const char * description = "");
	QString cvtVectorToString(const std::vector<cv::DMatch> vector, const char * description = "");
}

#endif // CVLIBWRAPPER_H
