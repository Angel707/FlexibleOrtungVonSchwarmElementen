#include "camerasetting.h"
#include "ui_camerasetting.h"

#include <QtDebug>
#include <assert.h>

CameraSetting::CameraSetting(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CameraSetting)
{
	ui->setupUi(this);
}

CameraSetting::~CameraSetting()
{
	delete ui;
}

void CameraSetting::getPoints(
	std::vector<cv::Point2f> & fourPixelPoints,
	std::vector<cv::Point2f> & fourObjectPoints
	)
{
	fourPixelPoints.push_back(cv::Point2f(ui->imageP0_x->value(), ui->imageP0_y->value()));
	fourPixelPoints.push_back(cv::Point2f(ui->imageP1_x->value(), ui->imageP1_y->value()));
	fourPixelPoints.push_back(cv::Point2f(ui->imageP2_x->value(), ui->imageP2_y->value()));
	fourPixelPoints.push_back(cv::Point2f(ui->imageP3_x->value(), ui->imageP3_y->value()));

	fourObjectPoints.push_back(cv::Point2f(ui->worldP0_x->value(), ui->worldP0_y->value()));
	fourObjectPoints.push_back(cv::Point2f(ui->worldP1_x->value(), ui->worldP1_y->value()));
	fourObjectPoints.push_back(cv::Point2f(ui->worldP2_x->value(), ui->worldP2_y->value()));
	fourObjectPoints.push_back(cv::Point2f(ui->worldP3_x->value(), ui->worldP3_y->value()));
}

void CameraSetting::getProjection(
	cv::Mat_<float> & pixelToWorld_Projection,
	cv::Mat_<float> & worldToPixel_Projection
	)
{
	std::vector<cv::Point2f> fourPixelPoints;
	std::vector<cv::Point2f> fourObjectPoints;
	getPoints(fourPixelPoints, fourObjectPoints);
	assert(fourPixelPoints.size() == fourObjectPoints.size());
	assert(fourPixelPoints.size() == 4);
	assert(fourObjectPoints[0] == cv::Point2f(0,0));

	pixelToWorld_Projection = cv::getPerspectiveTransform(fourPixelPoints, fourObjectPoints);
	worldToPixel_Projection = cv::getPerspectiveTransform(fourObjectPoints, fourPixelPoints);
	assert(pixelToWorld_Projection.size() == cv::Size(3,3));
	assert(worldToPixel_Projection.size() == cv::Size(3,3));

	//qDebug() << "-" << this->objectName() << "getProjection:"
	//		 << CVLibWrapper::cvtMatrixToString(pixelToWorld_Projection, "Pixel 2 World");
	//qDebug() << "-" << this->objectName() << "getProjection:"
	//		 << CVLibWrapper::cvtMatrixToString(worldToPixel_Projection, "World 2 Pixel");

	/*
	// Überprüfe Transformation: anhand eines cv::Mat image
	cv::Mat testImage = image.clone();
	cv::Mat imageBirdsView;

	for (uint i = 1; i < fourPixelPoints.size(); i++)
	{
		cv::Point2f begin = fourPixelPoints[i-1];
		cv::Point2f end = fourPixelPoints[i];
		cv::Point2f mid = end - begin;
		cv::circle(testImage, mid, 3, cv::Scalar(255,0,0));
	}

	cv::warpPerspective(testImage, imageBirdsView, Z0Plane_projection, testImage.size());

	for (uint i = 1; i < fourPixelPoints.size(); i++)
	{
		cv::Point2f begin = fourObjectPoints[i-1];
		cv::Point2f end = fourObjectPoints[i];
		cv::Point2f mid = end - begin;
		cv::circle(imageBirdsView, mid, 3, cv::Scalar(0,255,0));
	}

	cv::cvtColor(testImage, testImage, CV_BGR2RGB);
	cv::cvtColor(imageBirdsView, imageBirdsView, CV_BGR2RGB);

	// Qt Interface
	CameraPixmap * pItem;
	assert(camItem->childItems().size() >= 8);
	// Zeichne Polygone (Contours) in Background
	// erzeuge QGraphicPolygonItem's aus den Konturen
	pItem = dynamic_cast<CameraPixmap *>(camItem->childItems().at(7));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(testImage));
	pItem->setHomographyMatrix(Z0Plane_projection, "mm");
	pItem->show2DCoordinates();

	pItem = dynamic_cast<CameraPixmap *>(camItem->childItems().at(9));
	assert(pItem);
	pItem->setPixmap(Camera::cvtImageToPixmap(imageBirdsView));
	pItem->setEinheitPixel("mm");
	pItem->setHomographyMatrix(Z0Plane_backprojection, "Px");
	pItem->show2DCoordinates();
	*/
}


void CameraSetting::on_btnClose_clicked()
{
	qDebug() << "<(signal)<" << this->objectName() << "on_btnClose_clicked:"
			 << "Camera Setting fertig..";
	emit confirmed();
}
