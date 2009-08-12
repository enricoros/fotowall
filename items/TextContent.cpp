/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
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
#include "ButtonItem.h"
#include "CPixmap.h"
#include "RenderOpts.h"
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMimeData>
#include <QPainter>
#include <QTextDocument>
#include <QTextFrame>
#include <QUrl>
#include <QDebug>

TextContent::TextContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_text(0)
    , m_margin(4)
    , m_textRect(0, 0, 0, 0)\
{
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));

    // create a text document
    m_text = new QTextDocument(this);
#if QT_VERSION >= 0x040500
    m_margin = m_text->documentMargin();
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
}

TextContent::~TextContent()
{
    delete m_text;
}

QString TextContent::toHtml() const
{
    return m_text->toHtml();
}

void TextContent::setHtml(const QString & htmlCode)
{
    qWarning() << "SHTML";
    m_text->setHtml(htmlCode);
    updateTextConstraints();
    qWarning() << "/SHTML";
}

bool TextContent::fromXml(QDomElement & pe)
{
    // FIRST load text properties
    // NOTE: order matters here, we don't want to override the size restored later
    QString text = pe.firstChildElement("html-text").text();
    setHtml(text);

    AbstractContent::fromXml(pe);

    // load other values
    // ...
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

    // Save item position and size
    domElement = doc.createElement("html-text");
    pe.appendChild(domElement);
    text = doc.createTextNode(m_text->toHtml());
    domElement.appendChild(text);
}

QPixmap TextContent::renderAsBackground(const QSize & size, bool keepAspect) const
{
    // get the base empty pixmap
    QPixmap pix = AbstractContent::renderAsBackground(size, keepAspect);
    QSize textSize = boundingRect().size().toSize();
    const float w = size.width(),
                h = size.height(),
                tw = textSize.width(),
                th = textSize.height();
    if (w < 2 || h < 2 || tw < 2 || th < 2)
        return pix;

    // draw text (centered, maximized keeping aspect ratio)
    float scale = qMin(w / (tw + 16), h / (th + 16));
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

void TextContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void TextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    //QTextDocumentLayout * layout = qobject_cast<QTextDocumentLayout *>(m_text->documentLayout());
    //m_text->documentLayout();
    // the layout might need to expand the root frame to
    // the viewport if NoWrap is set
    //if (layout)
    //    layout->setViewport(boundingRect().toRect());

    // scale painter for adapting the Text Rect to the Contents Rect
    QRect cRect = contentsRect();
    painter->save();
    painter->translate(cRect.topLeft());
    if (m_textRect.width() > 0 && m_textRect.height() > 0) {
        qreal xScale = (qreal)cRect.width() / (qreal)m_textRect.width();
        qreal yScale = (qreal)cRect.height() / (qreal)m_textRect.height();
        if (!qFuzzyCompare(xScale, 1.0) || !qFuzzyCompare(yScale, 1.0))
            painter->scale(xScale, yScale);
    }

#if 0
    // standard rich text document drawing
    QAbstractTextDocumentLayout::PaintContext pCtx;
    m_text->documentLayout()->draw(painter, pCtx);
#else
    // manual drawing
    QPointF blockPos = -m_textRect.topLeft();

    // 1. for each Text Block
    int blockRectIdx = 0;
    for (QTextBlock tb = m_text->begin(); tb.isValid(); tb = tb.next()) {
        if (!tb.isVisible() || blockRectIdx > m_blockRects.size())
            continue;

        // 1.1. compute text insertion position
        const QRect & blockRect = m_blockRects[blockRectIdx++];
        QPointF iPos = blockPos - blockRect.topLeft();
        blockPos += QPointF(0, blockRect.height());

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
            //painter->setPen(QColor::fromHsv(qrand() % 360, 255, 255));
            painter->setBrush(Qt::NoBrush);
            QFontMetrics metrics(font);

            // 1.2.2. draw each character
            QString text = frag.text();
            foreach (const QChar & textChar, text) {
                painter->drawText(iPos, textChar);
                iPos += QPointF(metrics.width(textChar), 0);
            }
        }
    }
#endif

    //if (layout)
    //    layout->setViewport(QRect());

    painter->restore();
}

void TextContent::updateTextConstraints()
{
    // 1. actual content stretch
    double prevXScale = 1.0;
    double prevYScale = 1.0;
    if (m_textRect.width() > 0 && m_textRect.height() > 0) {
        QRect cRect = contentsRect();
        prevXScale = (qreal)cRect.width() / (qreal)m_textRect.width();
        prevYScale = (qreal)cRect.height() / (qreal)m_textRect.height();
    }

    // 2. LAYOUT TEXT. find out Block rects and Document rect
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

            QFontMetrics metrics(frag.charFormat().font());
            QString text = frag.text();
            if (text.trimmed().isEmpty())
                continue;

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
    m_textRect.adjust(-m_margin, -m_margin, m_margin, m_margin);

    // 3. resize content keeping stretch
    int w = (int)(prevXScale * (qreal)m_textRect.width());
    int h = (int)(prevYScale * (qreal)m_textRect.height());
    resizeContents(QRect(-w / 2, -h / 2, w, h));
}
