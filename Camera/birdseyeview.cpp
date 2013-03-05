#include "birdseyeview.h"

#include <QtDebug>
#include <assert.h>

BirdsEyeView::BirdsEyeView(QWidget * parent) :
	QGraphicsView(parent)
{
	// Interne Eigenschaften
	scene = new QGraphicsScene();
	assert(scene);

	// Rahmen
	border = scene->addRect(QRectF(0, 0, 0, 0));
	assert(border);
	border->hide();
	borderMustInit = true;

	// Clickbares Label
	connect( this, SIGNAL(clicked()), this, SLOT(slotClicked()) );

	// Eigenschaften
	this->setScene(scene);
	this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
}

void BirdsEyeView::mouseMoveEvent(QMouseEvent * mouseEvent)
{
	mouseEvent->accept();
	QPointF scenePos = this->mapToScene(mouseEvent->pos());
	QPointF objectPos = scenePos / 10; // Scene-Coord. -> Object-Coord.
	QString mousePos = QString("(%1,%2)Px = (%3,%4,0)cm").arg(
				QString("%1").arg(scenePos.x()),
				QString("%1").arg(scenePos.y()),
				QString("%1").arg(objectPos.x()),
				QString("%1").arg(objectPos.y())
				);
	this->setToolTip(mousePos);
}

void BirdsEyeView::initBorder(
	qreal width, qreal height)
{
	if (!borderMustInit) return;
	// "positives Spielfeld" (0->max)
	border->setRect(0,0,width,height);

	QPen p = QPen();
	p.setStyle(Qt::SolidLine);
	p.setWidth(3);
	p.setBrush(QColor(0,0,0,230)); // RGBA
	//p.setCapStyle(Qt::RoundCap);
	//p.setJoinStyle(Qt::RoundJoin);
	border->setPen(p);

	QBrush b = QBrush();
	b.setColor(QColor(0,0,200,100)); // RGBA
	b.setStyle(Qt::SolidPattern);
	border->setBrush(b);

	border->setX(0);
	border->setY(0);

	border->show();
	borderMustInit = false;
}

void BirdsEyeView::setNoVisiblePolygon(
	QPolygonF polygon
	)
{

}

void BirdsEyeView::recordingUpdate()
{
	this->update();
}

void BirdsEyeView::mouseReleaseEvent(QMouseEvent *)
{
	//emit clicked();
}

void BirdsEyeView::slotClicked()
{
	qDebug() << "Frame Clicked with ID";

	qDebug() << "END: Frame Clicked with ID";
}

bool BirdsEyeView::addItem(SchwarmElementItem * item)
{
	assert(item);
	scene->addItem(item);
	item->show();
	objectList.push_back(item);
	return true;
}

// SLOT
void BirdsEyeView::addNewItem(SchwarmElementItem * item)
{
	assert(item);
	addItem(item);
}

// SLOT
bool BirdsEyeView::isNewItem(const QPoint & itemPos)
{
	QGraphicsItem * item = this->itemAt(itemPos);
	if (!item) return false;
	if (border->boundingRect() == item->boundingRect()) return false;
	return true;
}

void BirdsEyeView::updateItem(QGraphicsItem*item)
{

}
