#ifndef DETECTOBJECT_H
#define DETECTOBJECT_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>
#include "../Camera/Camera.h"
#include "../Camera/schwarmelement.h"
#include "../Camera/camerapixmap.h"
#include "../Camera/schwarmelementitem.h"

class DetectObject : public QGraphicsView
{
	Q_OBJECT
public:
	DetectObject(QWidget * parent = 0);
	~DetectObject(){}
	bool addCamera(Camera * cam);
	bool recordingUpdate();
	bool takePictureWithoutObject();
	bool takePictureWithObject();
	bool takeNextMovingCameraPicture();
	void calibrateZ0Plane(
		const std::vector<cv::Point2f> & fourPixelPoints,
		const std::vector<cv::Point2f> & fourObjectPoints);
	bool detectSchwarmElementContour();
	bool detect3DSchwarmElement(const cv::Point2f & nullPoint);
	//bool getCurrentImageAndDetectObject();
	void calibrateObjectsColor(bool showHist = false);
	bool trackObjectByHist();

	enum DO_OBJECTDETECTFUNCTION {
		DO_NOOBJECT = 0,
		DO_LED,
		DO_SCHWARMELEMENT
	};
	void setDetectObjectFunction(enum DO_OBJECTDETECTFUNCTION mode);


	bool analyseColorSpace();
	bool calibrateObjectsDescriptors(uint angleDegree);

signals:


public slots:


protected:
	void mouseMoveEvent(QMouseEvent *);

private:
	// Camera View Setup
	QGraphicsScene * scene;
	Camera * cam;
	QGraphicsItemGroup * camItem;
	int cam_width;
	int cam_maxheigth; // debug variable
	enum DO_OBJECTDETECTFUNCTION detectObjectFunktion;

	// Eigenschaften zum Kalibrieren der Z=0 - Ebene (Plane)
	std::vector<cv::Point2f> Z0Plane_fourPixelPoints;
	std::vector<cv::Point2f> Z0Plane_fourObjectPoints;
	cv::Mat Z0Plane_projection;
	cv::Mat Z0Plane_backprojection;

	// Eigenschaften zum Kalibrieren der Kontur des Objects
	// Image without object (background image)
	cv::Mat imageWithoutObject;
	// Image with object
	cv::Mat imageWithObject;
	std::vector<cv::Point> contourObject;

	cv::MatND objectHist;

	// Eigenschaften zum Ermitteln der 3D-Koordinaten (Rotierende Kamera)
	QList<cv::Mat> movingCamera;
	// Counter: Image with Object
	uint movingCameraCount;

	/* VERALTET
	bool detectObject(const cv::Mat & in, cv::Mat & out);
	bool detectLED(const cv::Mat & in, cv::Mat & out);
	bool detectSchwarmElement(const cv::Mat & in, cv::Mat & out);
	inline void pixelIsInRange(cv::Mat & image,
							  int row, int col, int channel,
							  double low, double high);
	*/

	SchwarmElement * object;
	SchwarmElementItem * objectItem;

	// Detect variables



	bool findContour_compareTwoImages(
		cv::Mat & imageDiff, std::vector<cv::Point> & objectContours,
		double strength);
	void findCorners(
		const cv::Mat & binaryImage, std::vector<cv::Point> & corners,
		double strength);



	void doWatershed(
		const cv::Mat & watershedInputImage, // color image
		const cv::Mat & binaryImg, // Histogramm-BackProjection
		cv::Mat & watershedBinaryImg, // 8UC1
		cv::Mat & segmentationGrayImg // 8UC1
		);

	void doHueBackProjection(
		const cv::Mat & colorImage, // reduced color image
		cv::Mat & backProjectionBinaryImg // backPorjection with Treshold
		);

	void doColorReduce(
		const cv::Mat & colorImage,
		cv::Mat & colorReducedImage,
		int div = 64
		);
};

#endif // DETECTOBJECT_H
