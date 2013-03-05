#include "cvlibwrapper.h"

namespace CVLibWrapper
{

// OpenCV cv::Mat types: 8U,16U,16S,32S,32F,64F
#define StrCV_8U  "CV_8U"
#define StrCV_16U "CV_16U"
#define StrCV_16S "CV_16S"
#define StrCV_32S "CV_32S"
#define StrCV_32F "CV_32F"
#define StrCV_64F "CV_64F"

#define CMP_CV_TYPES(thistype)		\
	(								\
	(thistype == CV_8U)?StrCV_8U:	\
	(thistype == CV_16U)?StrCV_16U:	\
	(thistype == CV_16S)?StrCV_16S:	\
	(thistype == CV_32S)?StrCV_32S:	\
	(thistype == CV_32F)?StrCV_32F:	\
	(thistype == CV_64F)?StrCV_64F:	\
	"unknown"						\
	)


	QString cvtMatrixToString(const cv::Mat & matrix, const char * description)
	{
		cv::Mat_<double> matrixd = matrix;
		QString str = QString("Matrix %1 (%2x%3)x%4 (type: %5):\n").arg(
					description,
					QString("%1").arg(matrixd.rows),
					QString("%1").arg(matrixd.cols),
					QString("%1").arg(matrixd.channels()),
					QString("%1").arg(CMP_CV_TYPES(matrixd.type()))
					);
		for (int i = 0; i < matrixd.rows; i++)
		{
			for (int j = 0; j < matrixd.cols; j++)
			{
				str.append(QString("%1").arg(matrixd(i,j)));
				if (j < matrixd.cols-1) str.append(",     ");
			}
			if (i < matrixd.rows-1) str.append(";\n");
		}
		return str;
	}

	QString cvtVectorToString(const std::vector<cv::Point2f> vector, const char * description)
	{
		QString str = QString("CV-Point2f-Vector %1 (Size %2):\n").arg(
							description,
							QString("%1").arg(vector.size())
							);
		for (uint i = 0; i < vector.size(); i++)
		{
			cv::Point2f p = vector[i];
			str.append(QString("(%1,     %2)\n").arg(
						QString("%1").arg(p.x),
						QString("%1").arg(p.y)
						));
		}
		return str;
	}

	QString cvtVectorToString(const std::vector<cv::DMatch> vector, const char * description)
	{
		QString str = QString("CV-DMatch-Vector %1 (Size %2):\n").arg(
					description,
					QString("%1").arg(vector.size())
					);
		str.append("(distance, imgIdx, queryIdx, trainIdx):\n");
		for (uint i = 0; i < vector.size(); i++)
		{
			cv::DMatch p = vector[i];
			str.append(QString("(%1,   %2, %3, %4)\n").arg(
						QString("%1").arg(p.distance),
						QString("%1").arg(p.imgIdx),
						QString("%1").arg(p.queryIdx),
						QString("%1").arg(p.trainIdx)
						));
		}
		return str;
	}
}
