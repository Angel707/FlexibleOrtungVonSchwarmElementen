/*
 * Camera.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: Angelos Drossos
 */

#include "Camera.h"
#include <iostream>
#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <cstring>
#include <time.h>

#include <QtDebug>
#include <assert.h>

Camera::Camera()
{
	camera_id = "NoName";
	this->setObjectName(QString::fromStdString(camera_id));
	camera_device = -1;
	isCalibrated = isRecording = isCalibrating = false;
	einheitPixel = "Px";
	einheitWorld = "mm";
	transformationActivated = false;
}

Camera::Camera(std::string id, int device)
	: camera_id(id), camera_device(device)
{
	isCalibrated = isRecording = isCalibrating = false;
	camera.open(device);
}

Camera::~Camera()
{
	if (camera.isOpened()) camera.release();
}

void Camera::setID(std::string id)
{
	camera_id = id;
	this->setObjectName(QString::fromStdString(camera_id));
}

bool Camera::setDevice(int device)
{
	camera_device = device;
	return activateDevice();
}

bool Camera::activateDevice()
{
	if (camera.isOpened()) return true;
	camera.open(camera_device);
	return (bool)camera.isOpened();
}

std::string Camera::readID()
{
	return (camera_id);
}

int Camera::readDevice()
{
	return (camera_device);
}

//##############################################################################
// Dieser Bereich unterstützt Module beim Umrechnen von Koordinaten
//##############################################################################

// SLOT function!
void Camera::setTransformation()
{
	qDebug() << ">(slot)>" << this->objectName() << "setTransformation:"
			 << "Versuche Transformationsmatrizen zu laden...";
	assert(settingWidget);
	settingWidget->getProjection(
				homographyPixelToWorld,
				homographyWorldToPixel
				);
	assert(!homographyPixelToWorld.empty());
	assert(!homographyWorldToPixel.empty());
	assert(homographyPixelToWorld.size() == cv::Size(3,3));
	assert(homographyWorldToPixel.size() == cv::Size(3,3));
	einheitWorld = "mm";
	einheitPixel = "Px";

	qDebug() << "-" << this->objectName() << "setTransformation:"
			 << CVLibWrapper::cvtMatrixToString(homographyPixelToWorld, "Pixel 2 World");
	qDebug() << "-" << this->objectName() << "setTransformation:"
			 << CVLibWrapper::cvtMatrixToString(homographyWorldToPixel, "World 2 Pixel");

	transformationActivated = true;
}

void Camera::transformPixel(
	const QPointF & itemPos, cv::Point2f & pointPixel,
	QString & einheitx, QString & einheity) const
{
	pointPixel.x = itemPos.x();
	pointPixel.y = itemPos.y();
	einheitx = "Px";
	einheity = "Px";
}

bool Camera::transformPixelToWorld(
	const QPointF & itemPos, cv::Point2f & world2D,
	QString & einheitx, QString & einheity) const
{
	if (!transformationActivated)
	{
		qWarning() << "*" << camera_id.c_str() << "transformPixelToWorld:"
				   << "Noch keine Pixel-2-World-Matrix vorhanden.";
		return false;
	}
	cv::Point3f cvItemPos = cv::Point3f(itemPos.x(), itemPos.y(), 1); // homogene Koordinaten
	assert(homographyPixelToWorld.cols == 3);
	cv::Mat_<float> temp = homographyPixelToWorld * cv::Mat_<float>(cvItemPos);

	// DEBUG
	//QString matrix = CVLibWrapper::cvtMatrixToString(temp);
	//qDebug() << "-" << camera_id << "transformPixel2D:" << matrix;

	assert(temp.size() == cv::Size(1,3));
	const double tx = temp(0,0);
	const double ty = temp(1,0);
	const double tw = temp(2,0);
	world2D = cv::Point2f(tx/tw, ty/tw);
	einheitx = "mm";
	einheity = "mm";
	return true;
}

bool Camera::transformWorldToPixel(
	const QPointF & itemPos, cv::Point2f & pixel2D,
	QString & einheitx, QString & einheity) const
{
	if (!transformationActivated) return false;
	cv::Point3f cvItemPos = cv::Point3f(itemPos.x(), itemPos.y(), 1); // homogene Koordinaten
	assert(homographyWorldToPixel.cols == 3);
	cv::Mat_<float> temp = homographyWorldToPixel * cv::Mat_<float>(cvItemPos);

	// DEBUG
	//QString matrix = CVLibWrapper::cvtMatrixToString(temp);
	//qDebug() << "-" << camera_id << "transformPixel2D:" << matrix;

	assert(temp.size() == cv::Size(1,3));
	const double tx = temp(0,0);
	const double ty = temp(1,0);
	const double tw = temp(2,0);
	pixel2D = cv::Point2f(tx/tw, ty/tw);
	einheitx = "Px";
	einheity = "Px";
	return true;
}

bool Camera::isTransformationActivated()
{
	return transformationActivated;
}

//##############################################################################


unsigned int Camera::readNumberOfCalibrationImages()
{
	return (calibrationImages.size());
}

bool Camera::readCalibrationImage(unsigned int id, cv::Mat & image)
{
	if (id > calibrationImages.size()) return false;
	image = calibrationImages.at(id);
	return true;
}

bool Camera::readCalibrationPixmap(unsigned int id, QPixmap & image)
{
	if (id > calibrationImages.size()) return false;
	cv::Mat cvImage = calibrationImages.at(id);
	image = QPixmap::fromImage(QImage((const uchar *)(cvImage.data),
								   cvImage.cols,
								   cvImage.rows,
								   QImage::Format_RGB888));
	return true;
}

bool Camera::isOpened(void)
{
	return (bool)camera.isOpened();
}

bool Camera::isCalibratedCamera(void)
{
	return isCalibrated;
}

bool Camera::readImage(cv::Mat & image)
{
	if (!camera.grab())
	{
		return false;
	}
	if (!camera.retrieve(image))
	{
		return false;
	}
	return true;
}

bool Camera::initCalibration(unsigned int w, unsigned int h, unsigned int image_count)
{
	if ((w*h) * image_count < 2) return false;
	calib.init(w,h);
	return true;
}

bool Camera::testCalibrationShot(cv::Mat & image)
{
	cv::Mat gray;
	if (image.empty())
	{
		qDebug() << "image empty!";
		if (!camera.grab())
		{
			qDebug() << "grab failed!";
			return false;
		}
		if (!camera.retrieve(image))
		{
			qDebug() << "receive failed!";
			return false;
		}
	}
	cv::cvtColor(image, gray, CV_RGB2GRAY);
	if (!calib.tryCalibrationImage(gray, true, &image))
	{
		qDebug() << "calibration image not found!";
		return false;
	}
	return true;
}

bool Camera::takeCalibrationShot(cv::Mat & image)
{
	cv::Mat gray;
	if (image.empty())
	{
		qDebug() << "image empty!";
		if (!camera.grab())
		{
			qDebug() << "grab failed!";
			return false;
		}
		if (!camera.retrieve(image))
		{
			qDebug() << "receive failed!";
			return false;
		}
	}
	cv::cvtColor(image, gray, CV_RGB2GRAY);
	if (!calib.addCalibrationImage(gray, true, &image))
	{
		qDebug() << "calibration image not found!";
		return false;
	}
	return true;
}

bool Camera::doCalibration(const std::vector<cv::Mat> & calibList)
{
	// Kalibrierpunkte aus Bilder auslesen
	int n = (int) calibList.size();
	for (int i = 0; i < n; i++)
	{
		cv::Mat gray;
		cv::Mat currImg = calibList.at(i);
		cv::cvtColor(currImg, gray, CV_RGB2GRAY);
		if (!calib.addCalibrationImage(gray))
		{
			qDebug() << "Camera:" << "doCalibration:" << "no Calibration Image!";
			return false;
		}
	}
	// Kalibration durchführen
	if (!calib.calibrateCam(cameraMatrix, distributionCoefficients,
					   rotationVector, translationVector))
	{
		qDebug() << "Camera:" << "doCalibration:" << "Calibration failed!";
		return false;
	}
	// Kalibration Data
	calibrationImages = calibList;
	isCalibrated = true;
	return true;
}

void Camera::recordingStart(bool isCalibrating)
{
	isRecording = true;
	this->isCalibrating = isCalibrating;
}

void Camera::recordingStop()
{
	isRecording = isCalibrating = false;
}

// SLOT: Warte auf Signal, dass alle Kameras gegrabbt haben
void Camera::recordingAllGrabsFinished()
{
	qDebug() << ">(slot)>" << this->objectName() << "recordingAllGrabsFinished:"
			 << "alle Kameras haben ein neues Bild bezogen (Grab).";

	// Hole das neue Bild in den Speicher
	bool succ = recordingUpdate_RetrieveImage();
	if (!succ)
	{
		qCritical() << "!!" << this->objectName() << "recordingAllGrabsFinished:"
					<< "Das neue Bild konnte nicht komplett bezogen werden.";
		return;
	}

	qDebug() << "<(signal)<" << this->objectName() << "recordingAllGrabsFinished:"
			 << "Hier steht neues Bild zur Verfuegung..";
	emit newImage(this);
}

bool Camera::recordingUpdate_Full()
{
	if (!recordingUpdate_Grab()) return false;
	return recordingUpdate_RetrieveImage();
}

bool Camera::recordingUpdate_Grab()
{
	if (!isRecording) return false;
	if (!camera.grab())
	{
		qDebug() << camera_id.c_str() << "grab failed!";
		return false;
	}
	return true;
}

bool Camera::recordingUpdate_RetrieveImage()
{
	if (!isRecording) return false;
	if (!camera.retrieve(current_image))
	{
		qDebug() << camera_id.c_str() << "receive failed!";
		return false;
	}
	if (isCalibrating)
	{
		calibration_image = current_image.clone();
		isCalibrationImage = testCalibrationShot(calibration_image);
	}
	return true;
}

bool Camera::getRecordedImage(cv::Mat & image)
{
	if (current_image.empty()) return false;
	image = current_image.clone();
	return true;
}

bool Camera::getRecordedCalibration(cv::Mat & img, cv::Mat & img_calib, bool & isGoodImage)
{
	if ((!isCalibrating) || (calibration_image.empty())) return false;
	img = current_image.clone();
	img_calib = calibration_image.clone();
	isGoodImage = isCalibrationImage;
	return true;
}

bool Camera::getRecordedPixmap(QPixmap & image)
{
	if (current_image.empty())
	{
		qCritical() << "Camera:" << "getRecordedPixmap:" << "Camera is not recording!";
		return false;
	}
	cv::Mat cvImage;
	if (isCalibrating)
		cvImage = calibration_image.clone();
	else
		cvImage = current_image.clone();
	cv::cvtColor(cvImage,cvImage, CV_BGR2RGB);
	image = QPixmap::fromImage(QImage((const uchar *)(cvImage.data),
								   cvImage.cols,
								   cvImage.rows,
								   QImage::Format_RGB888));
	return true;
}

bool Camera::getRecordedUndistortedImage(cv::Mat & image)
{
	if (current_image.empty()) return false;
	cv::undistort(current_image, image,
			  cameraMatrix, distributionCoefficients);
	return true;
}

bool Camera::getRecordedUndistortedPixmap(QPixmap & image)
{
	if (!isCalibrated)
	{
		qCritical() << "Camera:" << "getRecordedUndistortedPixmap:" << "Camera is not calibrated!";
		return false;
	}
	if (current_image.empty())
	{
		qCritical() << "Camera:" << "getRecordedUndistortedPixmap:" << "Camera is not recording!";
		return false;
	}
	// entzerren
	cv::Mat undistorted;
	cv::Mat distorted = current_image.clone();
	cv::undistort(distorted, undistorted,
			  cameraMatrix, distributionCoefficients);

	/*
	// Objekterkennung
	cv::Mat hsv, luv, gray;
	cv::cvtColor(undistorted, hsv, CV_BGR2HSV);
	cv::cvtColor(undistorted, luv, CV_BGR2Luv);
	cv::cvtColor(undistorted, gray, CV_BGR2GRAY);

	std::vector<cv::Mat> hsv_ch, hsv_ch_after;
	cv::split(hsv, hsv_ch);
	cv::Mat h = hsv_ch.at(0);
	cv::Mat s = hsv_ch.at(1);
	cv::Mat v = hsv_ch.at(2);
	cv::inRange(h,100,200,h);
	cv::inRange(s,0.8,1,s);
	cv::inRange(v,0.2,1,v);
	hsv_ch_after.push_back(h);
	hsv_ch_after.push_back(s);
	hsv_ch_after.push_back(v);
	cv::merge(hsv_ch_after, hsv);
	cv::cvtColor(hsv,undistorted,CV_HSV2RGB);
	//cv::cvtColor(v,undistorted,CV_GRAY2RGB);
	*/

	// cv::Mat -> QPixmap
	cv::cvtColor(undistorted,undistorted, CV_BGR2RGB);
	image = QPixmap::fromImage(QImage((const uchar *)(undistorted.data),
								   undistorted.cols,
								   undistorted.rows,
								   QImage::Format_RGB888));
	return true;
}

bool Camera::searchExtrinsics(cv::Mat & absdiffImage)
{
	if (absdiffImage.empty())
	{
		qCritical() << "Camera" << camera_id.c_str() << "searchExtrinsics:"
					<< "Image is empty!";
		return false;
	}
	cv::Mat image = current_image.clone();


	return true;
}

//! Speichert die Konfiguration in einer Datei
bool Camera::backupToFile()
{
	// Datei allocieren
	QString filename = QString::fromStdString(camera_id);
	filename.prepend("../");
	filename.append(QString(" Video%1.cam").arg(camera_device));
	qDebug() << "Speicher Camera Daten unter:" << filename;
	cv::FileStorage file(filename.toStdString().c_str(), FileStorage::WRITE);
	// Variablen speichern
	file << "id" << camera_id;
	file << "device" << camera_device;
	if (isCalibrated)
	{
		file << "isCalibrated" << 1;
		//file << "calibrationDate_GM" << calibTime;
		file << "cameraMatrix" << cameraMatrix;
		file << "distributionCoefficients" << distributionCoefficients;
		assert(rotationVector.size() == translationVector.size());
		assert(calibrationImages.size() == translationVector.size());
		int anzahlBilder = (int) rotationVector.size();
		assert(anzahlBilder >= 0);
		file << "numberOfCalibrationImages" << anzahlBilder;
		for (int i = 0; i < anzahlBilder; i++)
		{
			std::ostringstream rV; rV << "rotationVector " << (i+1);
			file << rV.str().c_str() << rotationVector.at(i);
			std::ostringstream tV; tV << "translationVector " << (i+1);
			file << tV.str().c_str() << translationVector.at(i);
		}
		for (int i = 0; i < anzahlBilder; i++)
		{
			std::ostringstream img; img << "Image " << (i+1);
			file << img.str().c_str() << calibrationImages.at(i);
		}
	}
	else
	{
		file << "isCalibrated" << 0;
	}
	// Datei aushängen
	file.release();
	qDebug() << "Camera Daten gespeichert.";
	return true;
}

//! Läd die Konfiguration aus einer Datei
bool Camera::restoreFromFile(std::string filename, bool loadCalibrationImages)
{
	qDebug() << "lade Camera Daten von:" << filename.c_str();
	// Datei allocieren
	cv::FileStorage file(filename.c_str(), FileStorage::READ);
	if (!file.isOpened())
	{
		qCritical() << "Camera:" << "restoreFromFile:" << "Cannot open file!";
		return false;
	}
	// Variablen speichern
	file["id"] >> camera_id;
	this->setObjectName(QString::fromStdString(camera_id));
	file["device"] >> camera_device;
	int isCalibratedInt = -1;
	file["isCalibrated"] >> isCalibratedInt;
	assert(isCalibratedInt >= 0);
	if (isCalibratedInt)
	{
		isCalibrated = true;
		//calibTime = (struct tm*) file["calibrationDate_GM"];
		//file << "calibrationDate_GM" << asctime(gmtime(&calibTime));
		file["cameraMatrix"] >> cameraMatrix;
		file["distributionCoefficients"] >> distributionCoefficients;
		int anzahlBilder = 0;
		file["numberOfCalibrationImages"] >> anzahlBilder;
		for (int i = 0; i < anzahlBilder; i++)
		{
			cv::Mat rVec, tVec;
			std::ostringstream rV; rV << "rotationVector " << (i+1);
			std::ostringstream tV; tV << "translationVector " << (i+1);
			file[rV.str().c_str()] >> rVec;
			file[tV.str().c_str()] >> tVec;
			rotationVector.push_back(rVec);
			translationVector.push_back(tVec);
			if (loadCalibrationImages)
			{
				cv::Mat Image;
				std::ostringstream img; img << "Image " << (i+1);
				file[img.str().c_str()] >> Image;
				calibrationImages.push_back(Image);
			}
		}
	}
	else
	{
		isCalibrated = false;
	}
	// Datei aushängen
	file.release();
	qDebug() << "Camera Daten geladen:";
	qDebug() << "Camera-ID:" << camera_id.c_str();
	qDebug() << "Camera-Device:" << camera_device;
	qDebug() << "Camera is calibrated:" << isCalibrated;
	if (isCalibrated)
	{
		qDebug() << "Number of calibration images:" << calibrationImages.size();
	}
	return true;
}

bool Camera::getCameraMatrix(cv::Mat & k) const
{
	if (!isCalibrated) return false;
	k = cameraMatrix;
	return true;
}

bool Camera::getDistCoeff(cv::Mat & d) const
{
	if (!isCalibrated) return false;
	d = distributionCoefficients;
	return true;
}

bool Camera::getEssentialMatrix(
	const cv::Mat & fundamental, cv::Mat & essential) const
{
	if (!isCalibrated) return false;
	const cv::Mat & k = cameraMatrix;

	// fundamental: 3x3 (rows-by-cols-matrix)
	assert(fundamental.rows == fundamental.cols && fundamental.rows == 3);
	// camera matrix: 3xi
	assert(k.rows == 3);

	// E = K^T * F * K ,  K: Kamera matrix (3x3)
	// (ixi) = ((ix3) * (3x3)) * (3xi) = (ix3) * (3xi)
	essential = k.t() * fundamental * k;

	assert(essential.rows == k.cols && essential.cols == k.cols);
	return true;
}

void Camera::setSettingWidget(CameraSetting * setting)
{
	assert(setting);
	settingWidget = setting;
	settingWidget->setObjectName(QString("%1: Setting").arg(camera_id.c_str()));
	settingWidget->setWindowTitle(settingWidget->objectName());
	// Setze Transformationsmatrix, sobald das SettingsWidget mit den
	// Umrechnungskoordinaten geschlossen (confirmed) wird.
	bool succ = connect(
				settingWidget, SIGNAL(confirmed()),
				this, SLOT(setTransformation())
				);
	assert(succ);
}

CameraSetting * Camera::setting() const
{
	return settingWidget;
}

//===================================================================
// STATIC
// ------------------------------------------------------------------

Camera * Camera::QOpenCalibrationFile(QWidget * parent,
	bool * notEnoughMemory,
	bool * abortSelection,
	bool * loadingError)
{
	qDebug() << "-" << "Camera" << "QOpenCalibrationFile:"
				<< "Versuche Kamera zu oeffnen...";
	if (notEnoughMemory) *notEnoughMemory = false;
	if (abortSelection) *abortSelection = false;
	if (loadingError) *loadingError = false;
	Camera * cam = new Camera();
	CameraSetting * setting = new CameraSetting(0);
	if (!cam || !setting)
	{
		if (notEnoughMemory) *notEnoughMemory = true;
		qCritical() << "!!" << "Camera" << "QOpenCalibrationFile:"
					<< "Could not load Camera." << "Not enough memory.";
		return NULL;
	}
	qDebug() << "-" << "Camera" << "QOpenCalibrationFile:"
				<< "Camera und CameraSetting Objekt erfolgreich erzeugt.";

	// Load Calibrated Camera from File
	QString fileName =
			QFileDialog::getOpenFileName(parent,
										 QString("Open Camera Calibration"), // Titel
										 "../CameraData", // Dir
										 QString("Camera Calibration Files (*.cam)")); // Filter
	if (fileName.isEmpty())
	{
		if (abortSelection) *abortSelection = true;
		qCritical() << "*" << "Camera" << "QOpenCalibrationFile:"
					<< "Could not load Camera" << cam->camera_id.c_str()
					<< "from device no" << cam->readDevice()
					<< QString("(/dev/video%1).").arg(cam->readDevice())
					<< "User abort / empty filename.";
		delete cam;
		return NULL;
	}
	qDebug() << "-" << "Camera" << "QOpenCalibrationFile:"
				<< "Lade Kamera von:" << fileName;
	// Open File
	if (!cam->restoreFromFile(fileName.toAscii().data(), false))
	{
		if (loadingError) *loadingError = true;
		qCritical() << "!!" << "Camera" << "QOpenCalibrationFile:"
					<< "Could not load Camera" << cam->camera_id.c_str()
					<< "from device no" << cam->readDevice()
					<< QString("(/dev/video%1)").arg(cam->readDevice());
		delete cam;
		return NULL;
	}
	qDebug() << "-" << "Camera" << "QOpenCalibrationFile:"
				<< "Uebergebe das SettingWidget an die Kamera.";
	cam->setSettingWidget(setting);
	qDebug() << "-" << "Camera" << "QOpenCalibrationFile:"
				<< "Kamera Objekt vollstaendig geladen.";
	return cam;
}

QPixmap Camera::cvtImageToPixmap(const cv::Mat & image)
{
	// cv::Mat -> QPixmap
	return QPixmap::fromImage(QImage((const uchar *)(image.data),
								   image.cols,
								   image.rows,
								   QImage::Format_RGB888));
}

// aller erster Kalibrierungstest.. noch vor Qt-Interface
// Wichtig: Qt-Gui und das Gui von OpenCV können nicht parallel betrieben werden!!
// cv::namedWindow(winname); ist ein GUI von OpenCV.
bool Camera::test_camera(unsigned int delay, int device_no, int channel)
{
	printf("Starte Kamera Test-Routine:\n");
	cv::VideoCapture camera;
	printf("Versuche Kamera %d zu oeffnen..\n", device_no);
	camera.open(device_no);
	printf("Teste das erfolgreiche Oeffnen der Kamera %d..\n", device_no);
	if (!camera.isOpened())
	{
		printf("Es konnte keine Kamera gefunden werden.\n");
		printf("Ist \\dev\\video%d vorhanden?\n", device_no);
		printf("Kamera Test-Routine nicht erfolgreich beendet.\n");
		return false;
	}
	printf("Kamera %d gefunden und erfolgreich geoeffnet.\n", device_no);

	printf("------------------------\n");
	printf("Kamera Zustand:\n");
	printf("Position der Frames:    %f\n", camera.get(CV_CAP_PROP_POS_FRAMES));
	printf("Avi Ratio (0 bis 1):    %f\n", camera.get(CV_CAP_PROP_POS_AVI_RATIO));
	printf("Frame (width x height): %.0f x %.0f\n",
			camera.get(CV_CAP_PROP_FRAME_WIDTH), camera.get(CV_CAP_PROP_FRAME_HEIGHT));
	printf("FPS:                    %3.2f\n", camera.get(CV_CAP_PROP_FPS));
	printf("Codec (4-char-code):    %f\n", camera.get(CV_CAP_PROP_FOURCC));
	printf("Max Number of Frames:   %f\n", camera.get(CV_CAP_PROP_FRAME_COUNT));
	printf("Matrix Format:          %f\n", camera.get(CV_CAP_PROP_FORMAT));
	printf("Current Capture Mode:   %f\n", camera.get(CV_CAP_PROP_MODE));
	printf("Helligkeit:             %f\n", camera.get(CV_CAP_PROP_BRIGHTNESS)); // cameras only
	printf("Kontrast:               %f\n", camera.get(CV_CAP_PROP_CONTRAST)); // cameras only
	printf("Saturation:             %f\n", camera.get(CV_CAP_PROP_SATURATION)); // cameras only
	printf("Hue:                    %f\n", camera.get(CV_CAP_PROP_HUE)); // cameras only
	printf("Gain:                   %f\n", camera.get(CV_CAP_PROP_GAIN)); // cameras only
	printf("Exposure:               %f\n", camera.get(CV_CAP_PROP_EXPOSURE)); // cameras only
	printf("Convert to RGB:         %f\n", camera.get(CV_CAP_PROP_CONVERT_RGB));
	printf("------------------------\n");

	char *winname;
	asprintf(&winname, "Kamera %d - Channel %d", device_no, channel);
	cv::namedWindow(winname);

	printf("Starte Bildschleife (beliebige Taste zum Beenden druecken)..\n");
	bool stop(false);
	while(!stop)
	{
		// Kamera-Bild übertragen
		bool succ = camera.grab();
		if (!succ)
		{
			printf("Es konnte kein Bild mehr eingelesen werden!\n");
			break;
		}
		// Kamera-Bild laden
		cv::Mat frame;
		succ = camera.retrieve(frame, channel);
		if (!succ)
		{
			printf("Kamera-Channel %d existiert nicht oder konnte nicht eingelesen werden.\n", channel);
			printf("Kamera Test-Routine nicht erfolgreich beendet.\n");
			return false;
		}

		cv::imshow(winname, frame);
		if (cv::waitKey(delay) >= 0)
			stop = true;
	}
	printf("Kamera wird freigegeben..\n");
	camera.release();
	printf("Kamera Test-Routine erfolgreich beendet.\n");
	return true;
}

// *******************************************************
// VERALTET
// *******************************************************

bool Camera::calibrationWithChessboard(unsigned int w, unsigned int h)
{
	if (!camera.isOpened()) return false;
	char *winname;
	asprintf(&winname, "Kamera %d Calib", camera_device);
	cv::namedWindow(winname);

	// Calibration-Object erstellen
	if (w <= 0 || h <= 0) w = h = 7;
	calib.init(w,h);
	printf("CalibObject erstellt.\n");
	int count = 0;

	// Variablen für FPS-Simulators
	double duration = 0;
	bool durationReset(true);

	while(!isCalibrated)
	{
		// FPS-Simulator: Start
		if (durationReset)
		{
			duration = static_cast<double>(cv::getTickCount());
			durationReset = false;
		}
		// Kamera-Bild übertragen
		camera.grab();
		bool succ = camera.grab();
		if (!succ)
		{
			printf("Es konnte kein Bild mehr eingelesen werden!\n");
			break;
		}
		// Kamera-Bild laden
		cv::Mat frame_rgb, frame_gray;
		succ = camera.retrieve(frame_rgb);
		if (!succ)
		{
			return false;
		}
		// Graubild erzeugen
		cv::cvtColor(frame_rgb, frame_gray, CV_RGB2GRAY);

		// Original Bild zeigen
		cv::imshow(winname, frame_rgb);


		// Vergangene Zeit in ms: 1/Frequenz = Zeit
		// Anzahl der Ticks seit letztem Vermerk geteilt durch die TickFrequenz
		//duration = (static_cast<double>(cv::getTickCount()) - duration) / cv::getTickFrequency();
		//if (duration > 1000)
		//{
		// FPS-Simulator: Resette Zeit beim nächsten Durchlauf
		//			durationReset = true;
		//}

		int key = waitKey(1);
		if (count >= 50 || key == 27)
			break;
		if (key == 'k')
		{
			// Calibration
			if (calib.addCalibrationImage(frame_gray, false)) count++;
			if (count >= 6)
			{
				isCalibrated = calib.calibrateCam(
						cameraMatrix, distributionCoefficients,
						rotationVector, translationVector);
				printf("Calibration: %s\n", isCalibrated?"Erfolgreich":"Fehlgeschlagen !");
			}
		}
	}
	/*
	if (isCalibrated)
	{
		time_t current_time; time(&current_time);
		calibTime = gmtime(&current_time);
		printf("Calibration Time: %s (GM)\n", asctime(calibTime));
		//
	}
	*/
	backupToFile();
	return isCalibrated;
}

// veraltet
/*
bool Camera::findSpielfeld()
{
	if (!isCalibrated) return false;
	if (!camera.isOpened()) return false;
	printf("Finde Spielfeld..\n");
	//int key = waitKey();
	cv::Mat spielfeld, spielfeld_entzerrt;
	if (!camera.grab()) return false;
	if (!camera.retrieve(spielfeld)) return false;
	cv::namedWindow("Original");
	cv::imshow("Original", spielfeld);
	cv::undistort(spielfeld, spielfeld_entzerrt, cameraMatrix, distributionCoefficients);
	cv::namedWindow("Entzerrt");
	cv::imshow("Entzerrt", spielfeld);
	calib.findOneMoreChess(spielfeld_entzerrt, homography);
	//calib.~CamCalibration();
	waitKey();
	return true;
}
*/

/*		// Threshold Image
		cv::Mat frame_th;
		cv::threshold(frame_gray,frame_th, 128, 200, CV_THRESH_BINARY);
		cv::imshow("Thres", frame_th);
 */

/*		// Bild bearbeiten: Sobel
		cv::Mat sobelxy, sobelx, sobely, sobel;
		cv::Sobel(frame_gray, sobelx, CV_16S, 1,0, 3,0.4);
		cv::Sobel(frame_gray, sobely, CV_16S, 0,1, 3,0.4);
		sobelxy = cv::abs(sobelx) + cv::abs(sobely);
		double sobmin, sobmax;
		cv::minMaxLoc(sobelxy, &sobmin, &sobmax);
		sobelxy.convertTo(sobel, CV_8U, -255./sobmax, 255);
		// Sobel-Bild zeigen
		//cv::imshow("Sobel", sobel);
 */


/*		// Bild bearbeiten: Harris ecken
		cv::Mat corners, dilated, localMax;
		cv::cornerHarris(frame_gray, corners, 3, 3, 0.01);
		double minStr, maxStr;
		cv::minMaxLoc(corners, &minStr, &maxStr);
		cv::dilate(corners, dilated, cv::Mat());
		cv::compare(corners, dilated, localMax, cv::CMP_EQ);
		// Harris ecken-Bild zeigen
		//cv::imshow("Ecken von Harris", localMax);
 */
