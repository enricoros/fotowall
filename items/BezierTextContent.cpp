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
    // Fixme : set the real bounding rect !
    //setBoundingRect(QRect(0,0,300, 300));
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

void BezierTextContent::configureBezier()
{
    BezierTextDialog dial;
    if(dial.exec() == QDialog::Accepted) {
        m_font = dial.font();
        m_fontSize = dial.fontSize();
        m_path = dial.path();
        m_text = dial.text();
    }
}

// ::QGraphicsItem
void BezierTextContent::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mousePressEvent(event);
    if (event->button() == Qt::RightButton) {
        setSelected(true);
        configureBezier();
    }
}
void BezierTextContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
}
void BezierTextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    qDebug() << "BezierTextContent::paint() text << " << m_text;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->strokePath(m_path, QPen(Qt::red, 2));

    m_font.setPointSize(m_fontSize);
    painter->setFont(m_font);
    QFontMetricsF metrics(m_font);
    qreal curLen = 0;
    //qDebug()<<one<<two<<three<<four;
    for (int i = 0; i < m_text.length(); ++i) {
        qreal t = m_path.percentAtLength(curLen);
        QPointF pt = m_path.pointAtPercent(t);
        qreal angle = m_path.angleAtPercent(t);
        QString txt;
        txt.append(m_text[i]);
        painter->save();
        painter->translate(pt);
        //qDebug()<<"txt = "<<txt<<", angle = "<<angle<<curLen<<t;
        painter->rotate(angle);
        painter->drawText(0, 0, txt);
        painter->restore();

        qreal incremenet = metrics.width(txt);
        curLen += incremenet;
    }
}
