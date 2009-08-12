#include "BezierTextContent.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsItem>

#define BEZIER_MARGIN 40

BezierTextContent::BezierTextContent(QGraphicsScene *scene, QGraphicsItem *parent)
    : AbstractContent(scene, parent, false)
{
    qDebug() << "BezierTextContent::BezierTextContent()";
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));
    //m_text = tr("Bezier text");
    m_font.setPointSize(16);
}

void BezierTextContent::setText(const QString &text)
{
    m_text = text;
    update();
}
QString BezierTextContent::text() const
{
    return m_text;
}

void BezierTextContent::setFont(const QFont &font)
{
    m_font = font;
    update();
}
QFont BezierTextContent::font() const
{
    return m_font;
}

void BezierTextContent::setPath(const QPainterPath &path)
{
    m_path = path;
    QRectF rect = m_path.boundingRect();
    //QRectF rect = m_path.controlPointRect();
    qDebug() << "setPath, bounding rect << " << rect;
    resizeContents(QRect(0, 0, rect.width() + BEZIER_MARGIN, rect.height() + BEZIER_MARGIN), false);
    update();
}

void BezierTextContent::setFontSize(const int size)
{
    m_font.setPointSize(size);
    update();
}

void BezierTextContent::setControlPoints(const QList<QPointF> controlPts)
{
    m_controlPoints = controlPts;
}
QList<QPointF> BezierTextContent::controlPoints() const
{
    return m_controlPoints;
}

// ::AbstractContent
bool BezierTextContent::fromXml(QDomElement & parentElement)
{
}
void BezierTextContent::toXml(QDomElement & parentElement) const
{
}
//QPixmap BezierTextContent::renderAsBackground(const QSize & size, bool keepAspect) const
//{
//}

void BezierTextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);
    painter->setPen(Qt::black);
    if(m_text.isEmpty()) {
        painter->drawText(boundingRect().x(), boundingRect().y() + 30, tr("Right click to configure."));
        return;
    }

    qDebug() << "BezierTextContent::paint() text << " << m_text;
    qDebug() << "BezierTextContent::paint() content rect << " << boundingRect();
    painter->setRenderHint(QPainter::Antialiasing);

    //painter->strokePath(m_path, QPen(Qt::red, 2));
    painter->setFont(m_font);
    QFontMetricsF metrics(m_font);
    qreal curLen = 0;
    // Fixme : path positions are absolute frome the config dialog, and so position is not exact
    qDebug() << "at 0: " << m_path.pointAtPercent(0);
    QPointF resetOrigin = QPointF(m_path.boundingRect().x()-BEZIER_MARGIN/2, m_path.boundingRect().y()-BEZIER_MARGIN/2);
    for (int i = 0; i < m_text.length(); ++i) {
        qreal t = m_path.percentAtLength(curLen);
        QPointF pt = QPointF(m_path.pointAtPercent(t).x(), m_path.pointAtPercent(t).y()) - resetOrigin;
        qreal angle = -m_path.angleAtPercent(t);
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
    qDebug() << "Resize contents: "<< rect;
    if (!rect.isValid())
        return;
    AbstractContent::resizeContents(rect, keepRatio);
}
