#include "BezierTextContent.h"
#include "items/BezierTextDialog.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsItem>

BezierTextContent::BezierTextContent(QGraphicsScene *scene, QGraphicsItem *parent)
    : AbstractContent(scene, parent, false)
{
    qDebug() << "BezierTextContent::BezierTextContent()";
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));
}

void BezierTextContent::setText(const QString &text)
{
    m_text = text;
    update();
}
void BezierTextContent::setFont(const QFont &font)
{
    m_font = font;
    update();
}
void BezierTextContent::setPath(const QPainterPath &path)
{
    m_path = path;
    QRectF rect = m_path.boundingRect();
    resizeContents(QRect(0, 0, rect.width()+5, rect.height()+5));
    update();
}
void BezierTextContent::setFontSize(const int size)
{
    m_fontSize = size;
    update();
}

// ::AbstractContent
bool BezierTextContent::fromXml(QDomElement & parentElement)
{
}
void BezierTextContent::toXml(QDomElement & parentElement) const
{
}
QPixmap BezierTextContent::renderAsBackground(const QSize & size, bool keepAspect) const
{
}
int BezierTextContent::contentHeightForWidth(int width) const
{
}

// ::QGraphicsItem
void BezierTextContent::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    AbstractContent::mousePressEvent(event);
}
void BezierTextContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
}
void BezierTextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    qDebug() << "BezierTextContent::paint() text << " << m_text;
    painter->setRenderHint(QPainter::Antialiasing);

    m_font.setPointSize(m_fontSize);
    painter->setFont(m_font);
    QFontMetricsF metrics(m_font);
    qreal curLen = 0;
    for (int i = 0; i < m_text.length(); ++i) {
        qreal t = m_path.percentAtLength(curLen);
        QPointF pt = m_path.pointAtPercent(t);
        qDebug() << pt;
        qreal angle = m_path.angleAtPercent(t);
        QString txt;
        txt.append(m_text[i]);
        painter->save();
        painter->translate(pt);
        painter->rotate(angle);
        painter->drawText(0, 0, txt);
        painter->restore();

        qreal incremenet = metrics.width(txt);
        curLen += incremenet;
    }
}

void BezierTextContent::resizeContents(const QRect & rect, bool keepRatio)
{
    if (!rect.isValid())
        return;
    AbstractContent::resizeContents(rect, keepRatio);
}
