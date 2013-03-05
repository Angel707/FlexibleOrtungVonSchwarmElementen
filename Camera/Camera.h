/*
 * Camera.h
 *
 *  Created on: Aug 12, 2011
 *      Author: Angelos Drossos
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <iostream>
#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <cstring>

// OpenCV und Qt
#include <opencv2/opencv.hpp>
#include <QObject>
#include <QtGui>

// Eigene Includes
#include "CamCalibration.h"
#include "../CameraSetting/camerasetting.h"
#include "cvlibwrapper.h"

class Camera : public QObject
{
	Q_OBJECT
public: //=====================================================================
	Camera(); // id = "NoName", device = -1, call setID and setDevice!
	Camera(std::string id, int device = 0);
	virtual ~Camera();

	//! setzt die ID
	void setID(std::string id);

	//! setzt die Device-Nummer und versucht diese zu öffnen.
	bool setDevice(int device);

	void setSettingWidget(CameraSetting * setting);
	CameraSetting * setting() const;

	bool activateDevice();

	std::string readID();
	int readDevice();

	//! Prüft, ob die Kamera richtig initialisiert (geöffnet) werden konnte.
	bool isOpened(void);

	bool isCalibratedCamera(void);

	//!
	bool readImage(cv::Mat & image);

	//! initialisiert die Kalibration
	bool initCalibration(unsigned int w, unsigned int h, unsigned int image_count);

	//! Nimmt ein Bild zur Kalibration auf (wird entfernt!!!)
	bool takeCalibrationShot(cv::Mat & image);

	//! Testet das übergebene Bild, ob es zur Kalibration verwendet werden kann.
	bool testCalibrationShot(cv::Mat & image);

	//! Führt die Calibration durch, indem eine Liste an Bildern übergeben wird.
	//! Jedes Bild sollte mit testCalibrationShot(cv::Mat & image) vorher getestet
	//! worden sein, da diese nicht erneut überprüft werden.
	bool doCalibration(const std::vector<cv::Mat> & calibList);

	//! Berechnet die Essential Matrix E aus der Kamera Matrix K
	//! und der Fundamental Matrix F: E = K^T * F * K.
	//! Die Kamera muss dazu bereits kalibriert sein.
	//! Die Funktion gibt (nur) false zurück, falls die Kamera noch nicht kalibriert ist.
	bool getEssentialMatrix(const cv::Mat & fundamental, cv::Mat & essential) const;

	bool getCameraMatrix(cv::Mat & k) const;

	bool getDistCoeff(cv::Mat & distCoeff) const;

	//! Calibriert die Kamera und erzeugt
	bool calibrationWithChessboard(unsigned int weight = 0, unsigned int height = 0);

	//! Findet anhand einer kalibrierten Kamera das Spielfeld
	//bool findSpielfeld();//veraltet

	bool searchExtrinsics(cv::Mat & absdiffImage);

	//! Speichert die Konfiguration in einer Datei
	bool backupToFile();

	//! Läd die Konfiguration aus einer Datei
	bool restoreFromFile(std::string file, bool loadCalibrationImages = true);

	void recordingStart(bool isCalibration = false);
	void recordingStop();
	bool recordingUpdate_Full();
	bool recordingUpdate_Grab();
	bool recordingUpdate_RetrieveImage();
	bool getRecordedImage(cv::Mat & image);
	bool getRecordedPixmap(QPixmap & image);
	bool getRecordedCalibration(cv::Mat & original, cv::Mat & withPoints, bool & isGoodImage);
	bool getRecordedUndistortedImage(cv::Mat & image);
	bool getRecordedUndistortedPixmap(QPixmap & image);
	unsigned int readNumberOfCalibrationImages();
	bool readCalibrationImage(unsigned int id, cv::Mat & image);
	bool readCalibrationPixmap(unsigned int id, QPixmap & image);


	//! Convertiert das Bildformat von cv::Mat nach QPixmap.
	//! Ein RGB-Bild im Bildformat cv::Mat liegt typischerweise als BGR vor.
	//! Mit dem Paramter code kann das Bild vorher transformiert werden.
	//! Dabei wird die Funktion cv::cvtColor() aufgerufen, welcher der Parameter code
	//! übergeben wird.
	static QPixmap cvtImageToPixmap(const cv::Mat & image);

	//! Läd eine kalibrierte Kamera aus einer Datei, die bei der Kalibration abgespeichert wurde.
	//! Dabie wird ein Qt-Window geöffnet, in welchem man die Datei auswählen kann.
	//! Wenn nicht genug Speicherplatz zur Verfügung steht (*notEnoughMemory=true)
	//! oder die Auswahlaktion abgebrochen wird (*abortSelection=true)
	//! oder die Datei nicht korrekt geladen werden kann (*loadingError=true),
	//! so wird NULL zurückgegeben; ansonsten ein Zeiger auf die Kamera.
	//! Mit parent wird das Eltern-Widget angegeben, zu welchem das Qt-Window gehören soll.
	//! Wenn das Eltern-Widget geschlossen wird, so wird auch das Qt-Window geschlossen.
	//! Default ist "*this".
	static Camera * QOpenCalibrationFile(QWidget * parent = 0,
										 bool * notEnoughMemory = 0,
										 bool * abortSelection = 0,
										 bool * loadingError = 0);

	//! Testet eine Kamera: Die Kamera wird zuerst geöffnet, dann nach (möglichen) Einstellungen hin untersucht;
	//! danach wird eine Bildschleife durchlaufen und es werden solange Bilder geschossen, bis eine beliebige Taste gedrückt wird;
	//! am Ende wird die Kamera wieder freigegeben.
	//! Jeder Schritt wird in der Konsole ausgegeben.
	//! Beim Öffnen der Kamera und bei der Untersuchung der Einstellungen kann es
	//! zu internen Fehlermeldungen kommen. Denn nicht jede Kamera unterstützt alle Eigenschaften.
	//! Diese Funktion darf nicht in ein Qt-Gui geladen werden, da das Gui von OpenCV
	//! nicht mit dem von Qt zusammen arbeiten kann! Events können nicht geshared werden.
	//! @param delay Verzögerung in ms zwischen Bildaufnahme und Bildanzeige : 0 bedeutet Standbild.
	//! @param device_no Die Nummer des Kamera-Devices (beginnend bei 0).
	//! @param channel Falls die Kamera mehrere Channels besitzt, kann damit der Channel angegeben werden.
	//! @return true, falls die Kamera (Device und Channel) gefunden wurden; ansonsten false.
	static bool test_camera(unsigned int delay = 100, int device_no = 0, int channel = 0);


	//-------------------------------------------------------------------------
	// Dieser Bereich unterstützt Module beim Umrechnen von Koordinaten
	//-------------------------------------------------------------------------
	//! Erzeugt leserliche Koordinaten. Die Koordinaten bleiben bis auf einen Skalierungsfaktor erhalten.
	//! Es werden beispielsweise "mm" in "cm" umgerechnet, wenn es angebracht ist.
	void transformPixel(const QPointF & itemPos, cv::Point2f & pointPixel, QString & einheitx, QString & einheity) const;
	bool transformPixelToWorld(const QPointF & itemPos, cv::Point2f & pointWorld, QString & einheitx, QString & einheity) const;
	bool transformWorldToPixel(const QPointF & itemPos, cv::Point2f & pointPixel, QString & einheitx, QString & einheity) const;
	bool isTransformationActivated();

signals: //====================================================================
	//! Teilt allen Empfängern mit, dass Sie sich ein neues Bild laden können.
	void newImage(Camera * cam);

public slots: //===============================================================
	//! Hiermit kann der Kamera mitgeteilt werden,
	//! dass alle Kameras recordingUpdate_Grab() ausgeführt haben.
	//! Danach wird das Signal newImage(this) ausgegeben.
	void recordingAllGrabsFinished();

	//! Holt sich die Transformation aus dem SettingsWidget
	void setTransformation();

private: //====================================================================
	//-------------------------------------------------------------------------
	// Camera Kennung
	//-------------------------------------------------------------------------
	std::string camera_id;	// backup
	int camera_device;		// backup
	bool isCalibrated;		// backup
	CameraSetting * settingWidget;
	//struct tm *calibTime;

	//-------------------------------------------------------------------------
	// Camera Schnittstelle / Video Simulation
	//-------------------------------------------------------------------------
	cv::VideoCapture camera;	// not init.
	bool isRecording;			// false
	cv::Mat current_image;		// Mat()
	// Camera wird calibriert
	bool isCalibrating;			// false
	cv::Mat calibration_image;	// Mat()
	bool isCalibrationImage;	// false ?
	// Camera Parameter
	cv::Mat cameraMatrix;	// backup
	cv::Mat distributionCoefficients;	// backup

	//-------------------------------------------------------------------------
	// Calibrier-Daten
	//-------------------------------------------------------------------------
	CamCalibration calib;				// not init
	vector<cv::Mat> calibrationImages;	// backup
	vector<cv::Mat> rotationVector;		// backup
	vector<cv::Mat> translationVector;	// backup

	//-------------------------------------------------------------------------
	// Koordinatentransformation
	//-------------------------------------------------------------------------
	bool transformationActivated;
	cv::Mat_<float> homographyPixelToWorld;
	cv::Mat_<float> homographyWorldToPixel;
	QString einheitPixel;
	QString einheitWorld;

};

#endif /* CAMERA_H_ */
