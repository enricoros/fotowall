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
  , m_word(word)
{
    // customize item
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
    setPen(Qt::NoPen);

    // adapt font to the relative count
    const double maxFontSize = 96;
    double gamma = (double)(m_word.count - minCount) / (double)(maxCount - minCount);
    int fontSize = 8 + (int)(maxFontSize * gamma);
    m_font.setPointSize(fontSize);

    regeneratePath();
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
    path.addText(0, 0, m_font, m_word.variants.begin().key());

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
