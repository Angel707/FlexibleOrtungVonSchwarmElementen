#include "cameracalibrationwidget.h"
#include "ui_cameracalibrationwidget.h"

#include "../Camera/portopencvqt.h"
#include <sstream>

#include "matrixwidget.h"

CameraCalibrationWidget::CameraCalibrationWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CameraCalibrationWidget)
{
	ui->setupUi(this);
	/*
	cv::Mat A(10, 5, DataType<float>::type);
	MatrixWidget * m = new MatrixWidget(this);
	m->setGeometry(QRect(200,20, 100, 100));
	m->show();
	m->showMatrixCV(A);
	*/
}

CameraCalibrationWidget::~CameraCalibrationWidget()
{
	delete ui;
}

CalibrationFrame * CameraCalibrationWidget::createCalibrationFrame(unsigned int i)
{
	unsigned int const start_x = 150;
	unsigned int const start_y = 260;
	unsigned int const w = 160;
	unsigned int const h = 120;
	unsigned int const margin = 5;
	unsigned int const columns = std::floor((this->width()-start_x) / (w+margin));

	CalibrationFrame * calibFrame = new CalibrationFrame(i, &cam, this);
	calibFrame->setGeometry(QRect(start_x + (i%columns)*(w+margin),
							 start_y + std::floor(i/columns)*(h+margin),
							 w, h));
	calibFrame->show();
	return calibFrame;
}

void CameraCalibrationWidget::on_pushButtonChangeDevice_clicked()
{
	qDebug() << "Button" << ui->pushButtonChangeDevice->text() << "clicked!";

	// Camera-Daten aus Formular auslesen
	QString name = ui->lineEditName->text();
	int device_no = ui->spinBoxDevice->value();

	// Camera Initialisieren
	cam.setID(name.toStdString());
	if (!cam.setDevice(device_no))
	{
		qDebug() << "setDevice nicht erfolgreich";
		return;
	}
	qDebug() << "Camera-Init erfolgreich";

	// Calibration Pattern (Schachbrett-Muster) und Anzahl Bilder initialisieren
	int pw = ui->spinBoxChessWidth->value();
	int ph = ui->spinBoxChessHeigth->value();
	if (!cam.initCalibration(pw,ph,calibNumber))
	{
		qDebug() << "initCalibration nicht erfolgreich";
		return;
	}
	qDebug() << "Calibration-Pattern initialisiert";

	// Calibration Frames erstellen
	qDebug() << "Init calibList..";
	calibCounter = 0;
	calibNumber = ui->spinBoxBilderNo->value();
	for (unsigned int i=0; i < calibNumber; i++)
	{
		calibList.push_back(createCalibrationFrame(i));
	}

	// Button umbenennen
	ui->pushButtonCalibration->setEnabled(true);
	ui->pushButtonChangeDevice->setText("Reset Camera");
	qDebug() << "Button umbenannt nach:" << ui->pushButtonChangeDevice->text();

	// Video config
	cam.recordingStart(true);
	timerID = startTimer(10); // 10 ms Countdown, dann timerEvent() ausführen
	qDebug() << "END: Button" << ui->pushButtonChangeDevice->text() << "clicked!";
}

void CameraCalibrationWidget::on_pushButtonCalibration_clicked()
{
	qDebug() << "Button" << ui->pushButtonCalibration->text() << "clicked!";

	// Timer Stop
	killTimer(timerID);

	// Sind alle Calibration-Bilder gesetzt?
	bool isReadyToCalibrate = true;
	for (unsigned int i = 0; i < calibNumber; i++)
	{
		CalibrationFrame * calibFrame = calibList.at(i);
		if (!calibFrame->isReadyToCalibrate())
		{
			isReadyToCalibrate = false;
			qDebug() << "Not enough Calibration images";
			return;
		}
	}

	// Sammel die Bilder (ohne Markierung der erkannten Punkte) zur Kalibration
	std::vector<cv::Mat> images;
	for (unsigned int i = 0; i < calibNumber; i++)
	{
		CalibrationFrame * calibFrame = calibList.at(i);
		images.push_back(calibFrame->getImageCV());
	}
	qDebug() << "CameraCalibrationWidget:" << "on_pushButtonCalibration_clicked:" << "Anzahl Images:" << images.size();

	// Führe Kalibration aus
	if (!cam.doCalibration(images))
	{
		qDebug() << "CameraCalibrationWidget:" << "Calibration:" << "Fehlgeschlagen!";
		timerID = startTimer(10);
		return;
	}
	qDebug() << "CameraCalibrationWidget:" << "Calibration:" << "Erfolgreich :)";
	ui->pushButtonSave->setEnabled(true);
	cam.recordingStart(!cam.isCalibratedCamera());
	timerID = startTimer(10);
}

void CameraCalibrationWidget::timerEvent(QTimerEvent*) {
	// der Timer wird durch startTimer(int t) , t in ms, initialisiert
	// und timerEvent aufgerufen, wenn der Countdown bei 0 gelandet ist

	// neues Camera-Bild holen
	if (!cam.recordingUpdate_Full())
	{
		qDebug() << "Recording failed!";
	}

	// Camera-Bild an Video-Frame übergeben
	QPixmap image;
	if (!cam.getRecordedPixmap(image))
	{
		qDebug() << "CameraCalibrationWidget:" << "timerEvent:" << "get Video Frame:" << "Fehlgeschlagen!";
		return;
	}
	image = image.scaledToHeight(ui->videoFrame->height());
	ui->videoFrame->setPixmap(image);

	if (cam.isCalibratedCamera())
	{
		QPixmap better_image;
		if (!cam.getRecordedUndistortedPixmap(better_image))
		{
			qDebug() << "CameraCalibrationWidget:" << "timerEvent:" << "get Video Frame:" << "Fehlgeschlagen!";
			return;
		}
		better_image = better_image.scaledToHeight(ui->videoFrameUndistorted->height());
		ui->videoFrameUndistorted->setPixmap(better_image);
	}
}

void CameraCalibrationWidget::on_pushButtonSave_clicked()
{
	cam.backupToFile();
}

void CameraCalibrationWidget::on_pushButtonLoadCalib_clicked()
{
	// Select File
	QString fileName =
			QFileDialog::getOpenFileName(this,tr("Open Camera Calibration"), ".",
										 tr("Camera Calibration Files (*.yml)"));
	// Open File
	cam.restoreFromFile(fileName.toAscii().data());
	cam.activateDevice();

	// Camera-Daten in Formular schrieben
	ui->lineEditName->setText(cam.readID().c_str());
	ui->spinBoxDevice->setValue(cam.readDevice());

	// Button umbenennen
	ui->pushButtonChangeDevice->setText("Reset Camera");
	qDebug() << "Button umbenannt nach:" << ui->pushButtonChangeDevice->text();

	// Restore Calibration Frames
	calibNumber = cam.readNumberOfCalibrationImages();
	qDebug() << "CalibNumber:" << calibNumber;
	calibList.clear();
	for (unsigned int i = 0; i < calibNumber; i++)
	{
		CalibrationFrame * cF = createCalibrationFrame(i);
		cF->restoreCalibration();
		calibList.push_back(cF);
	}

	// Restore Video
	cam.recordingStart(!cam.isCalibratedCamera());
	timerID = startTimer(10);
}
