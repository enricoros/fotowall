/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TextContent.h"
#include "frames/Frame.h"
#include "items/BezierCubicItem.h"
#include "items/TextProperties.h"
#include "CPixmap.h"
#include "RenderOpts.h"
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMimeData>
#include <QPainter>
#include <QTextDocument>
#include <QTextFrame>
#include <QUrl>


TextContent::TextContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_text(0)
    , m_textRect(0, 0, 0, 0)
    , m_textMargin(4)
    , m_shapeEditor(0)
{
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));

    // create a text document
    m_text = new QTextDocument(this);
#if QT_VERSION >= 0x040500
    m_textMargin = m_text->documentMargin();
#endif

    // template text
    QFont font;
#ifdef Q_OS_WIN
    font.setFamily("Arial");
#endif
    font.setPointSize(16);
    m_text->setDefaultFont(font);
    m_text->setPlainText(tr("right click to edit..."));
    setHtml(m_text->toHtml());

    // shape editor
    m_shapeEditor = new BezierCubicItem(this);
    m_shapeEditor->setVisible(false);
    m_shapeEditor->setControlPoints(QList<QPointF>() << QPointF(-100, -50) << QPointF(-10, 40) << QPointF(100, -50) << QPointF(100, 50));
    connect(m_shapeEditor, SIGNAL(shapeChanged(const QPainterPath &)), this, SLOT(setShapePath(const QPainterPath &)));
}

TextContent::~TextContent()
{
    delete m_shapeEditor;
    delete m_text;
}

QString TextContent::toHtml() const
{
    return m_text->toHtml();
}

void TextContent::setHtml(const QString & htmlCode)
{
    m_text->setHtml(htmlCode);
    updateTextConstraints();
}

bool TextContent::hasShape() const
{
    return !m_shapePath.isEmpty();
}

void TextContent::clearShape()
{
    setShapePath(QPainterPath());
    setShapeEditing(false);
    emit notifyHasShape(false);
}

bool TextContent::isShapeEditing() const
{
    return m_shapeEditor->isVisible();
}

void TextContent::setShapeEditing(bool enabled)
{
    if (enabled) {
        // shape editor on
        if (!m_shapeEditor->isVisible()) {
            m_shapeEditor->show();
            emit notifyShapeEditing(true);
        }

        // begin new shape
        if (!hasShape()) {
            // use caching only when drawing shaped [disabled because updates are wrong when cached!]
            //setCacheMode(enabled ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);

            // use new shape
            setShapePath(m_shapeEditor->shape());
            emit notifyHasShape(true);
        }
    } else {
        // shape editor off
        if (m_shapeEditor->isVisible()) {
            m_shapeEditor->hide();
            emit notifyShapeEditing(false);
        }
    }
}

QWidget * TextContent::createPropertyWidget()
{
    TextProperties * p = new TextProperties();

    // common properties
    connect(p->bFront, SIGNAL(clicked()), this, SLOT(slotStackFront()));
    connect(p->bRaise, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
    connect(p->bLower, SIGNAL(clicked()), this, SLOT(slotStackLower()));
    connect(p->bBack, SIGNAL(clicked()), this, SLOT(slotStackBack()));
    connect(p->bDel, SIGNAL(clicked()), this, SIGNAL(deleteItem()), Qt::QueuedConnection);

    // shape properties
    p->bEditShape->setChecked(isShapeEditing());
    connect(this, SIGNAL(notifyShapeEditing(bool)), p->bEditShape, SLOT(setChecked(bool)));
    connect(p->bEditShape, SIGNAL(toggled(bool)), this, SLOT(setShapeEditing(bool)));
    p->bClearShape->setVisible(hasShape());
    connect(this, SIGNAL(notifyHasShape(bool)), p->bClearShape, SLOT(setVisible(bool)));
    connect(p->bClearShape, SIGNAL(clicked()), this, SLOT(clearShape()));

    return p;
}

bool TextContent::fromXml(QDomElement & pe)
{
    // FIRST load text properties and shape
    // NOTE: order matters here, we don't want to override the size restored later
    QString text = pe.firstChildElement("html-text").text();
    setHtml(text);

    // load default font
    QDomElement domElement;
    domElement = pe.firstChildElement("default-font");
    if (domElement.isElement()) {
        QFont font;
        font.setFamily(domElement.attribute("font-family"));
        font.setPointSize(domElement.attribute("font-size").toInt());
        m_text->setDefaultFont(font);
    }

    // load shape
    domElement = pe.firstChildElement("shape");
    if (domElement.isElement()) {
        bool shapeEnabled = domElement.attribute("enabled").toInt();
        domElement = domElement.firstChildElement("control-points");
        if (shapeEnabled && domElement.isElement()) {
            QList<QPointF> points;
            QStringList strPoint;
            strPoint = domElement.attribute("one").split(" ");
            points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
            strPoint = domElement.attribute("two").split(" ");
            points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
            strPoint = domElement.attribute("three").split(" ");
            points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
            strPoint = domElement.attribute("four").split(" ");
            points << QPointF(strPoint.at(0).toFloat(), strPoint.at(1).toFloat());
            m_shapeEditor->setControlPoints(points);
        }
    }

    // THEN restore the geometry
    AbstractContent::fromXml(pe);

    return true;
}

void TextContent::toXml(QDomElement & pe) const
{
    AbstractContent::toXml(pe);
    pe.setTagName("text");

    // save text properties
    QDomDocument doc = pe.ownerDocument();
    QDomElement domElement;
    QDomText text;

    // save text (in html)
    domElement = doc.createElement("html-text");
    pe.appendChild(domElement);
    text = doc.createTextNode(m_text->toHtml());
    domElement.appendChild(text);

    // save default font
    domElement = doc.createElement("default-font");
    domElement.setAttribute("font-family", m_text->defaultFont().family());
    domElement.setAttribute("font-size", m_text->defaultFont().pointSize());
    pe.appendChild(domElement);

    // save shape and control points
    QDomElement shapeElement = doc.createElement("shape");
    shapeElement.setAttribute("enabled", hasShape());
    pe.appendChild(shapeElement);
    if (hasShape()) {
        QList<QPointF> cp = m_shapeEditor->controlPoints();
        domElement = doc.createElement("control-points");
        shapeElement.appendChild(domElement);
        domElement.setAttribute("one", QString::number(cp[0].x())
                + " " + QString::number(cp[0].y()));
        domElement.setAttribute("two", QString::number(cp[1].x())
                + " " + QString::number(cp[1].y()));
        domElement.setAttribute("three", QString::number(cp[2].x())
                + " " + QString::number(cp[2].y()));
        domElement.setAttribute("four", QString::number(cp[3].x())
                + " " + QString::number(cp[3].y()));
    }
}

QPixmap TextContent::renderContent(const QSize & size, Qt::AspectRatioMode /*ratio*/) const
{
    // get the base empty pixmap
    QSize textSize = boundingRect().size().toSize();
    const float w = size.width(),
                h = size.height(),
                tw = textSize.width(),
                th = textSize.height();
    if (w < 2 || h < 2 || tw < 2 || th < 2)
        return QPixmap();

    // draw text (centered, maximized keeping aspect ratio)
    float scale = qMin(w / (tw + 16), h / (th + 16));
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    QPainter pixPainter(&pix);
    pixPainter.translate((w - (int)((float)tw * scale)) / 2, (h - (int)((float)th * scale)) / 2);
    pixPainter.scale(scale, scale);
    m_text->drawContents(&pixPainter);
    pixPainter.end();
    return pix;
}

int TextContent::contentHeightForWidth(int width) const
{
    // if no text size is available, use default
    if (m_textRect.width() < 1 || m_textRect.height() < 1)
        return AbstractContent::contentHeightForWidth(width);
    return (m_textRect.height() * width) / m_textRect.width();
}

void TextContent::selectionChanged(bool selected)
{
    // hide shape editing controls
    if (!selected && isShapeEditing())
        setShapeEditing(false);
}

void TextContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void TextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    // check whether we're drawing shaped
    const bool shapedPaint = hasShape() && !m_shapeRect.isEmpty();
    QPointF shapeOffset = m_shapeRect.topLeft();

    // scale painter for adapting the Text Rect to the Contents Rect
    QRect cRect = contentsRect();
    QRect sRect = shapedPaint ? m_shapeRect : m_textRect;
    painter->save();
    painter->translate(cRect.topLeft());
    if (sRect.width() > 0 && sRect.height() > 0) {
        qreal xScale = (qreal)cRect.width() / (qreal)sRect.width();
        qreal yScale = (qreal)cRect.height() / (qreal)sRect.height();
        if (!qFuzzyCompare(xScale, 1.0) || !qFuzzyCompare(yScale, 1.0))
            painter->scale(xScale, yScale);
    }

    // shape
    //const bool drawHovering = RenderOpts::HQRendering ? false : isSelected();
    if (shapedPaint)
        painter->translate(-shapeOffset);
    //if (shapedPaint && drawHovering)
    //    painter->strokePath(m_shapePath, QPen(Qt::red, 0));

#if 0
    // standard rich text document drawing
    QAbstractTextDocumentLayout::PaintContext pCtx;
    m_text->documentLayout()->draw(painter, pCtx);
#else
    // manual drawing
    QPointF blockPos = shapedPaint ? QPointF(0, 0) : -m_textRect.topLeft();

    // 1. for each Text Block
    int blockRectIdx = 0;
    for (QTextBlock tb = m_text->begin(); tb.isValid(); tb = tb.next()) {
        if (!tb.isVisible() || blockRectIdx > m_blockRects.size())
            continue;

        // 1.1. compute text insertion position
        const QRect & blockRect = m_blockRects[blockRectIdx++];
        QPointF iPos = shapedPaint ? blockPos : blockPos - blockRect.topLeft();
        blockPos += QPointF(0, blockRect.height());

        qreal curLen = 8;

        // 1.2. iterate over text fragments
        for (QTextBlock::iterator tbIt = tb.begin(); !(tbIt.atEnd()); ++tbIt) {
            QTextFragment frag = tbIt.fragment();
            if (!frag.isValid())
                continue;

            // 1.2.1. setup painter and metrics for text fragment
            QTextCharFormat format = frag.charFormat();
            QFont font = format.font();
            painter->setFont(font);
            painter->setPen(format.foreground().color());
            painter->setBrush(Qt::NoBrush);
            QFontMetrics metrics(font);

            // 1.2.2. draw each character
            QString text = frag.text();
            foreach (const QChar & textChar, text) {
                if (shapedPaint) {
                    // find point on shape and angle
                    qreal interval = metrics.width(textChar);
                    qreal t = m_shapePath.percentAtLength(curLen);
                    QPointF pt = m_shapePath.pointAtPercent(t);
                    qreal angle = -m_shapePath.angleAtPercent(t);

                    // draw rotated letter
                    painter->save();
                    painter->translate(pt);
                    painter->rotate(angle);
                    painter->drawText(iPos, textChar);
                    painter->restore();

                    curLen += interval;
                } else {
                    painter->drawText(iPos, textChar);
                    iPos += QPointF(metrics.width(textChar), 0);
                }
            }
        }
    }
#endif

    painter->restore();
}

QPainterPath TextContent::shapePath() const
{
    return m_shapePath;
}

void TextContent::setShapePath(const QPainterPath & path)
{
    if (path == m_shapePath)
        return;

    // invalidate rectangles
    m_textRect = QRect();
    m_shapeRect = QRect();

    // set new path
    m_shapePath = path;

    // regenerate text layouting
    updateTextConstraints();
}

void TextContent::updateTextConstraints()
{
    // 1. actual content stretch
    double prevXScale = 1.0;
    double prevYScale = 1.0;
   /* if (m_textRect.width() > 0 && m_textRect.height() > 0) {
        QRect cRect = contentsRect();
        prevXScale = (qreal)cRect.width() / (qreal)m_textRect.width();
        prevYScale = (qreal)cRect.height() / (qreal)m_textRect.height();
    }*/

    // 2. LAYOUT TEXT. find out Block rects and Document rect
    int minCharSide = 0;
    m_blockRects.clear();
    m_textRect = QRect(0, 0, 0, 0);
    for (QTextBlock tb = m_text->begin(); tb.isValid(); tb = tb.next()) {
        if (!tb.isVisible())
            continue;

        // 2.1.A. calc the Block size uniting Fragments bounding rects
        QRect blockRect(0, 0, 0, 0);
        for (QTextBlock::iterator tbIt = tb.begin(); !(tbIt.atEnd()); ++tbIt) {
            QTextFragment frag = tbIt.fragment();
            if (!frag.isValid())
                continue;

            QString text = frag.text();
            if (text.trimmed().isEmpty())
                continue;

            QFontMetrics metrics(frag.charFormat().font());
            if (!minCharSide || metrics.height() > minCharSide)
                minCharSide = metrics.height();

            // TODO: implement superscript / subscript (it's in charFormat's alignment)
            // it must be implemented in paint too

            QRect textRect = metrics.boundingRect(text);
            if (textRect.left() > 9999)
                continue;
            if (textRect.top() < blockRect.top())
                blockRect.setTop(textRect.top());
            if (textRect.bottom() > blockRect.bottom())
                blockRect.setBottom(textRect.bottom());

            int textWidth = metrics.width(text);
            blockRect.setWidth(blockRect.width() + textWidth);
        }
        // 2.1.B. calc the Block size of blank lines
        if (tb.begin() == tb.end()) {
            QFontMetrics metrics(tb.charFormat().font());
            int textHeight = metrics.height();
            blockRect.setWidth(1);
            blockRect.setHeight(textHeight);
        }

        // 2.2. add the Block's margins
        QTextBlockFormat tbFormat = tb.blockFormat();
        blockRect.adjust(-tbFormat.leftMargin(), -tbFormat.topMargin(), tbFormat.rightMargin(), tbFormat.bottomMargin());

        // 2.3. store the original block rect
        m_blockRects.append(blockRect);

        // 2.4. enlarge the Document rect (uniting the Block rect)
        blockRect.translate(0, m_textRect.bottom() - blockRect.top() + 1);
        if (blockRect.left() < m_textRect.left())
            m_textRect.setLeft(blockRect.left());
        if (blockRect.right() > m_textRect.right())
            m_textRect.setRight(blockRect.right());
        if (blockRect.top() < m_textRect.top())
            m_textRect.setTop(blockRect.top());
        if (blockRect.bottom() > m_textRect.bottom())
            m_textRect.setBottom(blockRect.bottom());
    }
    m_textRect.adjust(-m_textMargin, -m_textMargin, m_textMargin, m_textMargin);

    // 3. use shape-based rendering
    if (hasShape()) {
#if 1
        // more precise, but too close to the path
        m_shapeRect = m_shapePath.boundingRect().toRect();
#else
        // faster, but less precise (as it uses the controls points to determine
        // the path rect, instead of the path itself)
        m_shapeRect = m_shapePath.controlPointRect().toRect();
#endif
        minCharSide = qBound(10, minCharSide, 500);
        m_shapeRect.adjust(-minCharSide, -minCharSide, minCharSide, minCharSide);

        // FIXME: layout, save layouting and calc the exact size!
        //int w = m_shapeRect.width();
        //int h = m_shapeRect.height();
        //resizeContents(QRect(-w / 2, -h / 2, w, h));
        resizeContents(m_shapeRect);

  //      moveBy(m_shapeRect.left(), m_shapeRect.top());
//        m_shapePath.translate(-m_shapeRect.left(), -m_shapeRect.top());
        //setPos(m_shapeRect.center());
        return;
    }

    // 4. resize content keeping stretch
    int w = (int)(prevXScale * (qreal)m_textRect.width());
    int h = (int)(prevYScale * (qreal)m_textRect.height());
    resizeContents(QRect(-w / 2, -h / 2, w, h));
}

void TextContent::updateCache()
{
    /*
    m_cachePixmap = QPixmap(contentsRect().size());
    m_cachePixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter(&m_cachePixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    ...
    */
}
