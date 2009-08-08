#include "BezierTextContent.h"

BezierTextContent(QGraphicsScene *scene, QGraphicsItem *parent = 0)
    : AbstractContent(scene, parent, false)
{
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));
}

void setText(const QString &text)
{
}
// ::AbstractContent
bool fromXml(QDomElement & parentElement)
{
}
void toXml(QDomElement & parentElement) const
{
}
QPixmap renderAsBackground(const QSize & size, bool keepAspect) const
{
}
int contentHeightForWidth(int width) const
{
}

// ::QGraphicsItem
void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
}
void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0)
{
}
