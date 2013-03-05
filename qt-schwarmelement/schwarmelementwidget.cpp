#include "schwarmelementwidget.h"
#include "ui_schwarmelementwidget.h"

#include <QtDebug>
#include <assert.h>

// Hier wird der Timer in ms eingestellt.
#define UPDATE_SPEED 1

SchwarmElementWidget::SchwarmElementWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SchwarmElementWidget)
{
	ui->setupUi(this);
	timer = -1;
	objectImageTaken = objectImageTaken2 = backgoundImageTaken = changed = false;
}

SchwarmElementWidget::~SchwarmElementWidget()
{
	delete ui;
}

void SchwarmElementWidget::on_pushButtonAddCamera_clicked()
{
	qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked.";
	if (timer >= 0) killTimer(timer);

	// Spielfeld initialisieren
	ui->BirdsView->initBorder(ui->WorldWidth->value(), ui->WorldHeight->value());
	ui->WorldWidth->setReadOnly(true);
	ui->WorldHeight->setReadOnly(true);

	// Kamera laden
	Camera * cam = Camera::QOpenCalibrationFile(this);
	if (!cam)
	{
		qWarning() << "***" << this->objectName() << "on_pushButtonAddCamera_clicked:"
				 << "Kamera konnte nicht geladen werden. Bitte nochmal versuchen.";
		qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked END.";
		return;
	}
	qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked:"
			 << "Kamera geladen:" << cam->objectName();

	// Bild-Analyser laden
	SchwarmElementDetection * sed = new SchwarmElementDetection(this);
	if (!sed)
	{
		qCritical() << "!!" << this->objectName() << "on_pushButtonAddCamera_clicked:"
				 << "SchwarmElement-Detector konnte nicht geladen werden.";
		qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked END.";
		return;
	}

	// Kamera-Daten an Layout übergeben, damit diese sich aktualisieren können
	if (!ui->camView->addCamera(cam, sed))
	{
		qCritical() << "!!" << this->objectName() << "on_pushButtonAddCamera_clicked:"
				 << "Kamera konnte nicht der Camera-View hinzugefuegt werden!";
		qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked END.";
		return;
	}

	// Connections definieren
	connect(sed, SIGNAL(newItemFound(SchwarmElementItem*)),
			ui->BirdsView, SLOT(addNewItem(SchwarmElementItem*)));

	connect(sed, SIGNAL(newItemFound(QPoint)),
			ui->BirdsView, SLOT(isNewItem(QPoint)));

	connect(cam, SIGNAL(newImage(Camera*)),
			sed, SLOT(searchObjects(Camera*)));

	connect(ui->trackObject, SIGNAL(clicked()),
			sed, SLOT(analyse()));

	qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked:"
			 << "Kamera erfolgreich geladen.";

	// DEBUG: Teste SchwarmElementItem und die Speicherfunktion
	//k = new SchwarmElementItem("testSchwarmElement");
	//assert(k);
	//ui->BirdsView->addItem(k);
	//counter = 0;
	// DEBUG END

	timer = startTimer(UPDATE_SPEED);
	qDebug() << "-" << this->objectName() << "on_pushButtonAddCamera_clicked END.";
}

void SchwarmElementWidget::timerEvent(QTimerEvent*)
{
	ui->camView->recordingUpdate();

	// DEBUG: Teste SchwarmElementItem und die Speicherfunktion
	//counter++;
	//if (counter > 30)
	//	k->updatePosition(false);
	//else
	//	k->updatePosition(true, cv::Point3f(counter*4+90,90,3));
	// DEBUG END
}

