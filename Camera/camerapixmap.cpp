#include "camerapixmap.h"

#include <QtDebug>
#include <assert.h>

CameraPixmap::CameraPixmap(Camera * cam, QGraphicsItem *parent) :
	QGraphicsPixmapItem(parent)
{
	assert(cam);
	this->cam = cam;
	init();

	// Clickbares Label
	bool succ = connect( this, SIGNAL(clicked()), this, SLOT(slotClicked()) );
	assert(succ);

}

void CameraPixmap::slotClicked()
{
	qDebug() << "Frame Clicked with ID";
	// Unused..
	qDebug() << "END: Frame Clicked with ID";
}

void CameraPixmap::init()
{
	useWidthScaling = useHeightScaling = false;
	oldWidth = newWidth = 640;
	oldHeight = newHeight = 480;
	widthScale = heightScale = 1;
}

void CameraPixmap::setScene(QGraphicsScene & scene)
{
	scene.addItem(this);
}

void CameraPixmap::removeWidthScale()
{
	useWidthScaling = false;
}

void CameraPixmap::removeHeightScale()
{
	useHeightScaling = false;
}

void CameraPixmap::setWidthScale(qreal oldW, qreal newW)
{
	useWidthScaling = true;
	oldWidth = oldW;
	newWidth = newW;
	widthScale = newW / oldW;
}

void CameraPixmap::setHeightScale(qreal oldH, qreal newH)
{
	useHeightScaling = true;
	oldHeight = oldH;
	newHeight = newH;
	heightScale = newH / oldH;
}

void CameraPixmap::setPixmap(const QGraphicsPixmapItem * item)
{
	QPixmap pixmap = item->pixmap();
	if (pixmap.width() == newWidth && pixmap.height() == newHeight)
		QGraphicsPixmapItem::setPixmap(pixmap);
	else
	{
		setPixmap(pixmap, true);
	}
}

bool CameraPixmap::setPixmap(const QPixmap & pixmap, bool ignoreChangesInWidthOrHeight)
{
	if (!ignoreChangesInWidthOrHeight)
	{
		if ((useWidthScaling && pixmap.width() != oldWidth) ||
				(useHeightScaling && pixmap.height() != oldHeight))
		{
			qCritical() << this->objectName() << "setPixmap:"
						<< "Das Ausgangsbild hat sich geaendert"
						<< "von W,H:" << oldWidth << oldHeight
						<< "nach W,H:" << pixmap.width() << pixmap.height();
			return false;
		}
	}

	if (!useHeightScaling && !useWidthScaling)
	{
		QGraphicsPixmapItem::setPixmap(pixmap);
	}
	else
	{
		QPixmap scaledPixmap;
		if (useHeightScaling && !useWidthScaling)
			scaledPixmap = pixmap.scaledToHeight(newHeight);
		else if (!useHeightScaling && useWidthScaling)
			scaledPixmap = pixmap.scaledToWidth(newWidth);
		else // useHeightScaling && useWidthScaling
			scaledPixmap = pixmap.scaled(newWidth, newHeight, Qt::KeepAspectRatio);
		QGraphicsPixmapItem::setPixmap(scaledPixmap);
	}
	return true;
}

qreal CameraPixmap::getOldHeight() const
{
	return oldHeight;
}

qreal CameraPixmap::getOldWidth() const
{
	return oldWidth;
}

qreal CameraPixmap::getNewHeight() const
{
	return newHeight;
}

qreal CameraPixmap::getNewWidth() const
{
	return newWidth;
}

qreal CameraPixmap::getHeightScale() const
{
	return heightScale;
}

qreal CameraPixmap::getWidthScale() const
{
	return widthScale;
}

Camera * CameraPixmap::camera() const
{
	return cam;
}

void CameraPixmap::execMouseReleaseEvent()
{
	emit clicked();
}

//static function
CameraPixmap * CameraPixmap::isCameraPixmap(const QMouseEvent & event, const QGraphicsView & view)
{
	CameraPixmap * mitem = dynamic_cast<CameraPixmap *>(view.itemAt(event.pos()));
	if (!mitem) return NULL; // item ist kein CameraPixmap-Item
	return mitem;
}

bool CameraPixmap::handleMouseReleaseEvent(const QMouseEvent & event, const QGraphicsView & view)
{
	// CameraPixmap
	switch ( event.button() )
	{
	case Qt::LeftButton:
	{
		qDebug() << "-" << this->objectName() << "handleMouseReleaseEvent:"
				 << "Linke Maus losgelassen.";
		QPointF scenePos = view.mapToScene(event.pos());
		QPointF itemPos = this->mapFromScene(scenePos);
		cv::Point2f pixelPos;
		QString einheitX, einheitY;
		assert(this->camera());
		this->camera()->transformPixel(itemPos, pixelPos, einheitX, einheitY);
		QString pixel = QString("(%1%2,%3%4)").arg(
					QString("%1").arg(pixelPos.x),
					einheitX,
					QString("%1").arg(pixelPos.y),
					einheitY
					);
		qDebug() << "-" << this->objectName() << "handleMouseReleaseEvent:"
				 << "Geklickt auf (x,y):" << pixel;
	}
	break;

	case Qt::RightButton:
	{
		qDebug() << "<(signal)<" << this->objectName() << "handleMouseReleaseEvent:"
				 << "Rechte Maus losgelassen: ggf. Wechsel PixmapRetriever.";
		// Auf dieses Signal antworten nur verbundene Pixmaps (connect())
		emit switchPixmapRetriever(this);
	}
	break;

	case Qt::MidButton:
	{
		qDebug() << "-" << this->objectName() << "handleMouseReleaseEvent:"
				 << "Mittlere Maus losgelassen.";
		execMouseReleaseEvent();
	}
	break;

	default:
	break;
	}

	return true;
}

bool CameraPixmap::handleMouseMoveEvent(const QMouseEvent & event, const QGraphicsView & view)
{
	// CameraPixmap
	qDebug() << "-" << "CameraPixmap" << "handleMouseMoveEvent:"
			 << "Item ist CameraPixmap und Maus wird bewegt.";
	QPointF scenePos = view.mapToScene(event.pos());
	QPointF itemPos = this->mapFromScene(scenePos);
	if (itemPos.x() >= 0 && itemPos.y() >= 0)
	{
		assert(itemPos.x() < this->getNewWidth());
		assert(itemPos.y() < this->getNewHeight());
		itemPos.setX(itemPos.x() / this->getWidthScale());
		itemPos.setY(itemPos.y() / this->getHeightScale());
		assert(itemPos.x() < this->getOldWidth());
		assert(itemPos.y() < this->getOldHeight());

		QString tooltip;
		cv::Point2f pixelPos, worldPos;
		QString einheitX, einheitY;

		// Pixelkoordinaten
		assert(this->camera());
		this->camera()->transformPixel(itemPos, pixelPos, einheitX, einheitY);
		QString pixel = QString("(%1%2,%3%4)").arg(
					QString("%1").arg(pixelPos.x),
					einheitX,
					QString("%1").arg(pixelPos.y),
					einheitY
					);
		tooltip.append(pixel);

		// Weltkoordinaten
		if (this->camera()->transformPixelToWorld(itemPos, worldPos, einheitX, einheitY))
		{
			tooltip.append("\n");
			QString world2D = QString("(%1%2,%3%4)").arg(
						QString("%1").arg(worldPos.x),
						einheitX,
						QString("%1").arg(worldPos.y),
						einheitY
						);
			tooltip.append(world2D);
		}

		// Tooltip setzen
		this->setToolTip(tooltip);
	}
	else
	{
		qWarning() << "***" << this->objectName() << "mouseMoveEvent:"
				   << "negative Koordinate gefunden x,y:"
				   << itemPos.x() << itemPos.y();
		QString mousePos = QString("negativ(%1,%2)").arg(
					QString("%1").arg(itemPos.x()),
					QString("%1").arg(itemPos.y())
					);
		// Tooltip setzen
		this->setToolTip(mousePos);
	}
	return true;
}

// SLOT
void CameraPixmap::setUndistortedImage(Camera * cam)
{
	assert(cam == this->cam);
	qDebug() << ">(slot)>" << this->objectName() << "setUndistortedImage:"
			 << "Hole neues Bild von Kamera" << cam->objectName();
	QPixmap image;
	cam->getRecordedUndistortedPixmap(image);
	setPixmap(image);
}

// SLOT
void CameraPixmap::setAnalyseImageBGR(cv::Mat image)
{
	qDebug() << ">(slot)>" << this->objectName() << "setAnalyseImageBGR:"
			 << "Hole analysiertes Bild (BGR).";
	cv::cvtColor(image, image, CV_BGR2RGB);
	setPixmap(Camera::cvtImageToPixmap(image));
}

// SLOT
void CameraPixmap::setAnalyseImageGray(cv::Mat image) // or binary
{
	qDebug() << ">(slot)>" << this->objectName() << "setAnalyseImageGray:"
			 << "Hole analysiertes Bild (Gray).";
	cv::cvtColor(image, image, CV_GRAY2RGB);
	setPixmap(Camera::cvtImageToPixmap(image));
}
