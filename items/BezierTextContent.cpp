#include "BezierTextContent.h"
#include <QGraphicsScene>
#include <QGraphicsItem>

#define BEZIER_MARGIN 40

BezierTextContent::BezierTextContent(QGraphicsScene *scene, QGraphicsItem *parent)
    : AbstractContent(scene, parent, false)
{
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));
    m_text = tr("Bezier text");
    m_font.setPointSize(16);
    QList<QPointF> cpts;
    //Defaults control points
    cpts << QPointF(50, 50) << QPointF(140, 140) << QPointF(300, 250) << QPointF(300, 50);
    setControlPoints(cpts);
}

void BezierTextContent::setText(const QString &text)
{
    m_text = text;
    m_cachePixmap = QPixmap();
    update();
}
QString BezierTextContent::text() const
{
    return m_text;
}

void BezierTextContent::setFont(const QFont &font)
{
    m_font = font;
    m_cachePixmap = QPixmap();
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
    // Faster, but less precise (as it uses the controls points to determine the path rect,
    // instead of the path itself)
    //QRectF rect = m_path.controlPointRect();
    resizeContents(QRect(0, 0, rect.width() + BEZIER_MARGIN, rect.height() + BEZIER_MARGIN), false);
    m_cachePixmap = QPixmap();
    update();
}

void BezierTextContent::setFontSize(const int size)
{
    m_font.setPointSize(size);
    m_cachePixmap = QPixmap();
    update();
}

void BezierTextContent::setControlPoints(const QList<QPointF> controlPts)
{
    m_controlPoints = controlPts;
    if (controlPts.length() == 4) {
        m_path = QPainterPath(controlPts[0]);
        m_path.cubicTo(controlPts[1], controlPts[2], controlPts[3]);
    }
    m_cachePixmap = QPixmap();
    update();
}
QList<QPointF> BezierTextContent::controlPoints() const
{
    return m_controlPoints;
}

// ::AbstractContent
bool BezierTextContent::fromXml(QDomElement & parentElement)
{
    AbstractContent::fromXml(parentElement);

    // restore text
    QDomElement domElement;
    domElement = parentElement.firstChildElement("text");
    setFont(domElement.attribute("font-family"));
    setFontSize(domElement.attribute("font-size").toInt());
    setText(domElement.text());

    //restore path
    domElement = parentElement.firstChildElement("control-points");
    QStringList strPoint;
    QList<QPointF> points;
    strPoint = domElement.attribute("one").split(" ");
    points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
    strPoint = domElement.attribute("two").split(" ");
    points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
    strPoint = domElement.attribute("three").split(" ");
    points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
    strPoint = domElement.attribute("four").split(" ");
    points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
    setControlPoints(points);

    return true;
}
void BezierTextContent::toXml(QDomElement & parentElement) const
{
    AbstractContent::toXml(parentElement);
    parentElement.setTagName("bezier-text");

    // save text proparentElementrties
    QDomDocument doc = parentElement.ownerDocument();
    QDomElement domElement;
    QDomText text;

    // Save item position and size
    domElement = doc.createElement("text");
    parentElement.appendChild(domElement);
    domElement.setAttribute("font-family", m_font.family());
    domElement.setAttribute("font-size", m_font.pointSize());
    text = doc.createTextNode(m_text);
    domElement.appendChild(text);

    if (m_controlPoints.length() == 4) {
        domElement = doc.createElement("control-points");
        parentElement.appendChild(domElement);
        domElement.setAttribute("one", QString::number(m_controlPoints[0].x())
                + " " + QString::number(m_controlPoints[1].y()));
        domElement.setAttribute("two", QString::number(m_controlPoints[1].x())
                + " " + QString::number(m_controlPoints[0].y()));
        domElement.setAttribute("three", QString::number(m_controlPoints[2].x())
                + " " + QString::number(m_controlPoints[2].y()));
        domElement.setAttribute("four", QString::number(m_controlPoints[3].x())
                + " " + QString::number(m_controlPoints[3].y()));
    }
}

QPixmap BezierTextContent::renderAsBackground(const QSize & size, bool keepAspect) const
{
    return m_cachePixmap.scaled(size, keepAspect ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void BezierTextContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void BezierTextContent::updateCache()
{
    m_cachePixmap = QPixmap(contentsRect().size());
    m_cachePixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter(&m_cachePixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setFont(m_font);
    QFontMetricsF metrics(m_font);
    qreal curLen = 0;
    QPointF resetOrigin = QPointF(m_path.boundingRect().x()-BEZIER_MARGIN/2, m_path.boundingRect().y()-BEZIER_MARGIN/2);
    for (int i = 0; i < m_text.length(); ++i) {
        qreal t = m_path.percentAtLength(curLen);
        QPointF pt = QPointF(m_path.pointAtPercent(t).x(), m_path.pointAtPercent(t).y()) - resetOrigin;
        qreal angle = -m_path.angleAtPercent(t);
        QString txt;
        txt.append(m_text[i]);
        painter.save();
        painter.translate(pt);
        painter.rotate(angle);
        painter.drawText(0, 0, txt);
        painter.restore();

        qreal incremenet = metrics.width(txt);
        curLen += incremenet;
    }
}
void BezierTextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);
    if ( m_cachePixmap.isNull() ) {
        updateCache();
    }
    painter->drawPixmap(contentsRect(), m_cachePixmap);
}

