#include "calibrationframe.h"

#include <QtDebug>

CalibrationFrame::CalibrationFrame(int id, Camera * cam, QWidget * parent) :
	QLabel(parent)
{
	// Interne Eigenschaften
	this->id = id;
	this->cam = cam;
	isCalibrationImage = false;

	// Clickbares Label
	connect( this, SIGNAL(clicked()), this, SLOT(slotClicked()) );

	// Eigenschaften
	std::ostringstream stm; stm << "Calibration Image " << (id+1);
	this->setText(stm.str().c_str());
	this->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	this->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
}

void CalibrationFrame::mouseReleaseEvent(QMouseEvent *)
{
	emit clicked();
}

void CalibrationFrame::slotClicked()
{
	qDebug() << "Frame Clicked with ID " << id;
	// Aktuelles Bild holen
	if (!readImage())
	{
		qDebug() << "Frame lesen:" << "Nicht erfolgreich!";
		return;
	}
	qDebug() << "END: Frame Clicked with ID " << id;
}

bool CalibrationFrame::isReadyToCalibrate()
{
	return isCalibrationImage;
}

cv::Mat & CalibrationFrame::getImageCV()
{
	return image;
}

bool CalibrationFrame::readImage()
{
	// Lese aktuelles Camera-Bild aus
	cv::Mat anzeige;
	if (!cam->getRecordedCalibration(image, anzeige, isCalibrationImage))
	{
		qDebug() << "CalibrationFrame:" << "readImage failed!";
		return false;
	}
	qDebug() << "Is Calibration Image:" << isCalibrationImage;
	// Bild anzeigen
	setPixmapCV(anzeige);
	return true;
}

void CalibrationFrame::setPixmapCV(cv::Mat & image)
{
	QPixmap qpix = QPixmap::fromImage(QImage((const uchar *)(image.data),
								   image.cols,
								   image.rows,
								   QImage::Format_RGB888));
	qpix = qpix.scaledToHeight(this->height());
	this->setPixmap(qpix);
}

bool CalibrationFrame::restoreCalibration()
{
	QPixmap image;
	if (!cam->readCalibrationPixmap(id, image))
		return false;
	this->setPixmap(image);
	return true;
}
