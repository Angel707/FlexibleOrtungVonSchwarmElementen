#ifndef SCHWARMELEMENTITEM_H
#define SCHWARMELEMENTITEM_H

/*
  Diese Klasse dient dazu, SchwarmElemente als Item in einer QGraphicsView
  einzublenden. Dabei wird die Position des SchwarmElements intern gespeichert
  und auch in einer Datei (als Stream, je Zeile eine Positionsangabe) abgelegt.
  Es kann ein interner ObjektName vergeben werden sowie ein Mapping zu einer
  Mac-Adresse gemacht werden. Wenn noch kein Mapping vorhanden ist, so wird
  dies in den entsprechenden Funktionen mit dem Return=false mitgeteilt.

  Sollte eine Position nicht errechnet werden können, so muss entweder
  bei der Positionsaktualisierung positionIsKnown=false verwendet werden
  oder die aufrufende Funktion führt einfach kein Update durch.

  Beim Update wird intern die aktuelle Zeit festgehalten. In der Daten-Datei
  kann dieser auch nachgelesen werden: yyyy/MM/dd - hh:mm:ss.zzz;
  zzz sind Millisekunden.
*/

#include <QObject>
#include <QGraphicsItem>
#include <QtGui>
#include <opencv2/opencv.hpp>

#define UNKNOWN				cv::Point3f()
#define POSITION(x,y,z)		cv::Point3f((x),(y),(z))

class SchwarmElementItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	explicit SchwarmElementItem(const QString & objectName, QGraphicsItem * parent = 0);

	//! Der ObjektName bzw. die Objekt ID: Zugriff per objectName() bzw. setObjectName() [Vererbt]


	QRectF boundingRect() const;

	//! Zeichnet das Objekt in den gewünschten Farben:
	//! Grün, wenn das Objekt erkannt wurde
	//! Orange bis Rot, wenn das Objekt nicht erkannt wurde;
	//! Je röter, desto länger wurde es nicht erkannt;
	//! rot bedeutet: Objekt ist verschwunden.
	//! Auf diese Funktion greift die Klasse BirdsEyeView zu.
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

	// Identifizierung mit der Außenwelt über MAC
	bool mac(QString & mac) const;
	bool mac(ushort & mac1, ushort & mac2, ushort & mac3,
			 ushort & mac4, ushort & mac5, ushort & mac6);
	bool mapToMac(const ushort mac1, const ushort mac2, const ushort mac3,
				  const ushort mac4, const ushort mac5, const ushort mac6);
	bool isValidMac() const;

	bool updatePosition(bool positionIsKnown = false,
						const cv::Point3f & world = UNKNOWN);

	//! Speichert interne Daten des Objekts in einer Setup-Datei
	bool saveSetup();

	static bool isSchwarmElement(cv::Point2f & center,
								 cv::Point2f & left, cv::Point2f & top,
								 cv::Point2f & right, cv::Point2f & bottom );

signals:
	void redraw(QGraphicsItem * item);

public slots:

private:
	//! Zugehörige Mac-Adresse, "" == kein Mapping vorhanden
	QVector<ushort> macAddress;

	//! Setup Daten dieses Objekts
	cv::FileStorage file;
	QString filename_setup;
	//! DataRecord der Positionen
	QFile data;
	QString filename_data;
	//! Speichert das Anzeige-Format des TimeStamps
	QString timeFormat;

	//! Gibt an, wie lang die interne QVector-Länge betragen darf.
	//! Wenn sie zu lang wird, werden die ersten Einträge gelöscht.
	//! Kurzfristig werden (maxRecord+1) Einträge gespeichert.
	int maxRecord; // <=0: aus; Interner Parameter für die maximale Länge des QVectors

	//! Die Daten der Positionen
	QVector<QDateTime> timeStamps;
	QVector<cv::Point3f> worldPoints;
	//cv::Mat Image3D; // Idee verworfen
	QString worldEinheit;

	QRectF lastPosition;

	//! Aussehen des SchwarmElements
	static float const radius = 70; // in mm

	//! Nicht-Erkennungs-Counter
	bool detected;
	unsigned int notFound;
};

#endif // SCHWARMELEMENTITEM_H
