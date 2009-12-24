/***************************************************************************
 *                                                                         *
 *   This file is part of the Wordcloud project,                           *
 *       http://www.enricoros.com/opensource/wordcloud                     *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "WordItem.h"

#include <QApplication>
#include <QPainterPathStroker>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionFocusRect>
#include <QStyleOptionGraphicsItem>

using namespace Wordcloud;

WordItem::WordItem(const Word & word, const QFont & font, double rotation,
                   int minCount, int maxCount, QGraphicsItem * parent)
  : QAbstractGraphicsShapeItem(parent)
  , m_font(font)
  , m_rotation(rotation)
  , m_string(word.commonString)
{
    // customize item
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
    setPen(Qt::NoPen);

    // adapt font to the relative count
    const double maxFontSize = 96;
    double gamma = (double)(word.count - minCount) / (double)(maxCount - minCount);
    int fontSize = 8 + (int)(maxFontSize * gamma);
    m_font.setPointSize(fontSize);

    regeneratePath();
}

WordItem::WordItem(QDomElement & wordElement, QGraphicsItem * parent)
    : QAbstractGraphicsShapeItem(parent)
    , m_rotation((qreal)0.0)
{
    // customize item
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
    setPen(Qt::NoPen);

    // restore geometry
    QDomElement domElement;
    domElement = wordElement.firstChildElement("pos");
    qreal px = domElement.firstChildElement("x").text().toDouble();
    qreal py = domElement.firstChildElement("y").text().toDouble();
    setPos(px, py);

    // restore font
    domElement = wordElement.firstChildElement("font");
    QString family = domElement.firstChildElement("family").text();
    if (!family.isEmpty())
        m_font.setFamily(family);
    int pointSize = domElement.firstChildElement("pointSize").text().toInt();
    if (pointSize > 0)
        m_font.setPointSize(pointSize);

    // SIMPLE restore brush
    domElement = wordElement.firstChildElement("color");
    QColor col(domElement.text());
    setBrush(col.isValid() ? col : Qt::black);

    // rotation
    m_rotation = wordElement.firstChildElement("rotation").text().toDouble();

    // word
    m_string = wordElement.firstChildElement("string").text();

    // DO PATH!
    regeneratePath();
}

void WordItem::saveToXml(QDomElement & wordElement) const
{
    QDomDocument doc = wordElement.ownerDocument();
    QDomElement domElement;

    // save geometry
    domElement = doc.createElement("pos");
    QDomElement xElement = doc.createElement("x");
    QDomElement yElement = doc.createElement("y");
    xElement.appendChild(doc.createTextNode(QString::number(pos().x())));
    yElement.appendChild(doc.createTextNode(QString::number(pos().y())));
    domElement.appendChild(xElement);
    domElement.appendChild(yElement);
    wordElement.appendChild(domElement);

    // save font
    domElement = doc.createElement("font");
    QDomElement fElement = doc.createElement("family");
    QDomElement psElement = doc.createElement("pointSize");
    fElement.appendChild(doc.createTextNode(m_font.family()));
    psElement.appendChild(doc.createTextNode(QString::number(m_font.pointSize())));
    domElement.appendChild(fElement);
    domElement.appendChild(psElement);
    wordElement.appendChild(domElement);

    // SIMPLE save brush
    if (!brush().color().isValid())
        qWarning("WordItem::saveToXml: implement complex brush xml dump");
    domElement = doc.createElement("color");
    domElement.appendChild(doc.createTextNode(brush().color().name()));
    wordElement.appendChild(domElement);

    // rotation
    if (m_rotation != 0.0) {
        domElement = doc.createElement("rotation");
        domElement.appendChild(doc.createTextNode(QString::number(m_rotation)));
        wordElement.appendChild(domElement);
    }

    // word
    domElement = doc.createElement("string");
    domElement.appendChild(doc.createTextNode(m_string));
    wordElement.appendChild(domElement);
}

QVariant WordItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged)
        emit moved();
    return QAbstractGraphicsShapeItem::itemChange(change, value);
}

void WordItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    // draw the word
    painter->save();
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(m_path);

    // draw the standard selection
    if (option->state & QStyle::State_Selected) {
        QStyleOptionFocusRect opt;
        opt.rect = boundingRect().toRect();
        opt.backgroundColor = Qt::white;
        QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, painter, 0);
    }
    painter->restore();
}

QPainterPath WordItem::strokedPath() const
{
    // use path as-is if no pen or null
    const QPen & pen = this->pen();
    if (m_path == QPainterPath() || pen.widthF() <= 0.0)
        return m_path;

    // return enlarged (stroked) path
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    ps.setWidth(pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath fullPath = ps.createStroke(m_path);
    fullPath.addPath(m_path);
    return fullPath;
}

void WordItem::regeneratePath()
{
    // make the path from the text
    QPainterPath path;
    path.addText(0, 0, m_font, m_string);

    // rotate the text if needed
    if (m_rotation != 0.0) {
        QTransform pathForm;
        // perspective rotation
        //pathForm.rotate(-60 + (qrand() % 120), Qt::XAxis);
        //pathForm.rotate(-60 + (qrand() % 120), Qt::YAxis);
        // planar rotation
        pathForm.rotate(m_rotation, Qt::ZAxis);
        path = pathForm.map(path);
    }

    // center the path
    QPointF pathCenter = path.boundingRect().center();
#if QT_VERSION >= 0x040600
    path.translate(-pathCenter);
#elif QT_VERSION >= 0x040500
    path = QTransform::fromTranslate(-pathCenter.x(), -pathCenter.y()).map(path);
#else
    path = QTransform().translate(-pathCenter.x(), -pathCenter.y()).map(path);
#endif

    // update constants
    prepareGeometryChange();
    m_path = path;
    if (pen().widthF() <= 0.001)
        m_boudingRect = m_path.boundingRect();
    else
        m_boudingRect = strokedPath().boundingRect();
    update();
}
