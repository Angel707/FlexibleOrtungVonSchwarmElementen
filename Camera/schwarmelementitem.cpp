#include "schwarmelementitem.h"

#include <QtDebug>
#include <assert.h>



SchwarmElementItem::SchwarmElementItem(const QString & objectName, QGraphicsItem * parent) :
	QGraphicsItem(parent)
{
	this->setObjectName(objectName);
	this->setToolTip(objectName);
	timeFormat = QString("yyyy/MM/dd - hh:mm:ss.zzz");

	filename_setup = QString("%1/%2%3").arg(
				QString("../SchwarmElement-Data"),		// Path
				objectName,			// Filenname
				QString("-Setup.schwarm") // Extension
				);
	filename_data = QString("%1/%2%3").arg(
				QString("../SchwarmElement-Data"),		// Path
				objectName,			// Filenname
				QString("-Data.schwarm") // Extension
				);

	maxRecord = 0; // interner Counter
	worldEinheit = QString("mm");
	//radius = 70; // mm

	// Initial muss das Objekt erst gefunden werden
	detected = false;
	notFound = 0;

	qDebug() << "-" << this->objectName() << "SchwarmElementItem:"
			 << "Objekt Setup wird gespeichert unter:" << filename_setup;
	qDebug() << "-" << this->objectName() << "SchwarmElementItem:"
			 << "Objekt DataRecordings werden gespeichert unter:" << filename_data;

	data.setObjectName(filename_data);
	data.setFileName(filename_data);

	saveSetup();

	qDebug() << "-" << this->objectName() << "SchwarmElementItem:"
			 << "SchwarmElement erfolgreich erzeugt.";
}

// wichtige Funktion für QGraphicsItem
QRectF SchwarmElementItem::boundingRect() const
{
	if (worldPoints.empty()) return QRectF(0,0,0,0);
	return lastPosition; // Quadrat
}

// wichtige Funktion für QGraphicsItem
void SchwarmElementItem::paint(
	QPainter * painter,
	const QStyleOptionGraphicsItem * option,
	QWidget * ) // widget
{
	// Dies soll Performance bringen
	painter->setClipRect( option->exposedRect );

	// Aussehen der Kontur
	QPen p = QPen();
	p.setStyle(Qt::SolidLine);
	p.setWidth(3);
	p.setBrush(QColor(0,0,0,230));
	painter->setPen(p); // Qt::NoPen

	QRectF rect = boundingRect();
	if (rect.width() > 0 && rect.height() > 0)
	{
		// Aussehen des Inhalts
		QBrush b = QBrush();
		if (detected)
		{
			notFound = 0;
			b.setColor(QColor(0,255,0,255)); // RGBA
			b.setStyle(Qt::SolidPattern);
		}
		else
		{
			notFound++;
			// Hue: 60-gelb, 0-rot: 60-counter
			// Saturation: 0-"grau", 255-"stark farbig"
			// Value: 0-schwarz/dunkel (egal,was H/S ist), 255-hell
			const uint norm = 100;
			int hue = floor(notFound / norm);
			int sat = notFound%150+105;
			int val = 200;
			if ((60 - hue) < 0) hue = 0;
			QColor hsva; hsva.setHsv(hue, sat, val, 180);
			b.setColor(hsva); // HSVA
			b.setStyle(Qt::Dense4Pattern);
		}
		painter->setBrush(b);

		// Form -> Kontur und Inhalt
		QRectF rect = boundingRect();
		painter->drawEllipse(rect);
		painter->drawPoint(rect.center());

		QString pos = QString("%1: (%2%3,%4%5)").arg(
					this->objectName(),
					QString("%1").arg(rect.center().x()),
					"mm",
					QString("%1").arg(rect.center().y()),
					"mm"
					);
		this->setToolTip(pos);

		qDebug() << ">>>" << this->objectName() << "paint:"
				 << "Objekt ist gezeichnet worden.";

		this->update(rect);
	}
	else
	{
		qWarning() << "***" << this->objectName() << "paint:"
				   << "Objekt ist zu klein (Width=Height=0).";
		painter->setBrush(Qt::NoBrush);
	}
}

bool SchwarmElementItem::mac(QString & mac) const
{
	if (macAddress.size() == 0) return false;
	assert(macAddress.size() == 6);
	mac = QString("%1:%2:%3:%4:%5:%6").arg(
				QString("%1").arg(macAddress[0]),
				QString("%1").arg(macAddress[1]),
				QString("%1").arg(macAddress[2]),
				QString("%1").arg(macAddress[3]),
				QString("%1").arg(macAddress[4]),
				QString("%1").arg(macAddress[5])
				);
	return true;
}

bool SchwarmElementItem::mac(
	ushort & mac1, ushort & mac2, ushort & mac3,
	ushort & mac4, ushort & mac5, ushort & mac6)
{
	if (macAddress.size() == 0) return false;
	assert(macAddress.size() == 6);
	mac1 = macAddress[0];
	mac2 = macAddress[1];
	mac3 = macAddress[2];
	mac4 = macAddress[3];
	mac5 = macAddress[4];
	mac6 = macAddress[5];
	return true;
}

bool SchwarmElementItem::mapToMac(
	const ushort mac1, const ushort mac2, const ushort mac3,
	const ushort mac4, const ushort mac5, const ushort mac6)
{
	if (mac1 > 255 || mac2 > 255 || mac3 > 255 ||
			mac4 > 255 || mac5 > 255 || mac6 > 255)
		return false;
	macAddress.clear();
	assert(macAddress.size() == 0);
	macAddress.push_back(mac1);
	macAddress.push_back(mac2);
	macAddress.push_back(mac3);
	macAddress.push_back(mac4);
	macAddress.push_back(mac5);
	macAddress.push_back(mac6);
	assert(macAddress.size() == 6);
	return true;
}

bool SchwarmElementItem::isValidMac() const
{
	if (macAddress.size() == 0) return false;
	assert(macAddress.size() == 6);
	return true;
}

bool SchwarmElementItem::saveSetup()
{
	file.open(filename_setup.toStdString().c_str(), cv::FileStorage::WRITE);
	if (!file.isOpened())
	{
		qCritical() << "!!" << this->objectName() << "SchwarmElementItem:"
					<< "Datei" << filename_setup << "konnte nicht angelegt/geoeffnet werden!";
		return false;
	}
	file << "objectName" << objectName().toStdString().c_str();
	file << "maxRecord" << maxRecord;
	// TODO: weitere Eigenschaften hinzufügen..
	file.release();
	return true;
}


bool SchwarmElementItem::updatePosition(bool positionIsKnown,
	const cv::Point3f & world)
{
	// Zeit holen
	QDateTime now = QDateTime::currentDateTimeUtc(); // system clock in UTC

	qDebug() << "-" << this->objectName() << "updatePosition:"
				<< "Zeit:" << now.toString(timeFormat)
				<< "Position bekannt:" << ((positionIsKnown)?"ja":"nein");

	if (positionIsKnown && (world.x < radius || world.y < radius))
	{
		qWarning() << "*" << this->objectName() << "updatePosition:"
				   << "Das Objekt befindet sich zu dicht am Rand:"
				   << world.x << "oder" << world.y << "<" << radius;
		return false;
	}

	// Nur wenn Position bekannt ist, wird hinzugefügt.
	if (positionIsKnown)
	{
		// Management speichern
		timeStamps.push_back(now);
		worldPoints.push_back(world);
		assert(timeStamps.size() == worldPoints.size());

		// Last Position updaten
		lastPosition = QRectF(
					world.x - radius,
					world.y - radius,
					2*radius, 2*radius
					);

		this->setX(world.x - radius);
		this->setY(world.y - radius);

		qDebug() << "-" << this->objectName() << "updatePosition:"
					<< "Letzte Position (links oben):" << lastPosition.x() << lastPosition.y()
					<< "Groesse des Objekts:" << lastPosition.width() << lastPosition.height();

		// Ggf. QVector kürzen
		if (maxRecord > 0 && timeStamps.size() > maxRecord)
		{
			// Lösche alte Daten
			timeStamps.pop_front();
			worldPoints.pop_front();
			assert(timeStamps.size() == worldPoints.size());
		}
	}

	// Item updaten
	this->update(boundingRect());

	// In Data-Datei sichern: Auch unbekannte Positionen
	data.open(QIODevice::Append);
	if (!data.isOpen())
	{
		qCritical() << "!!" << this->objectName() << "updatePosition:"
					<< "Datei" << filename_data << "konnte nicht angelegt/geoeffnet werden!";
		return false;
	}
	if (positionIsKnown)
	{
		QTextStream out(&data);
		out << "TimeStamp_UTC: " << now.toString(timeFormat) << " "
			<< "Known Position: " << positionIsKnown << " "
			<< "World " << "(" << world.x << "," << world.y << "," << world.z << ")" << worldEinheit
			<< endl;
	}
	else
	{
		QTextStream out(&data);
		out << "TimeStamp_UTC: " << now.toString(timeFormat) << " "
			<< "Known Position: " << positionIsKnown
			<< endl;
	}
	data.close();

	return true;
}

// static
bool SchwarmElementItem::isSchwarmElement(cv::Point2f & center,
	cv::Point2f & left, cv::Point2f & top,
	cv::Point2f & right, cv::Point2f & bottom )
{
	float thres = 2*radius;
	if ((right.x	- left.x)	> (thres * 2)) return false;
	if ((bottom.y	- top.y)	> thres * 2) return false;
	if ((center.x	- left.x)	> thres) return false;
	if ((right.x	- center.x)	> thres) return false;
	if ((center.y	- bottom.y)	> thres) return false;
	if ((top.y		- center.y)	> thres) return false;
	qDebug() << "-" << "SchwarmElementItem" << "isSchwarmElement:"
			 << "Objekt gefunden :)";
	return true;
}
