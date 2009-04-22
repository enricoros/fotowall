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
#include "ButtonItem.h"
#include "CPixmap.h"
#include "RenderOpts.h"
#include "frames/Frame.h"
#include "richtexteditor_p.h"
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

TextContent::TextContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_text(0)
{
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));

    // create a text document
    m_text = new QTextDocument(this);
    QAbstractTextDocumentLayout * layout = m_text->documentLayout();
    connect(layout, SIGNAL(documentSizeChanged(const QSizeF &)), this, SLOT(slotTextResized(const QSizeF &)));

    // template text
    QFont font;
#ifdef Q_OS_WIN32
    font.setFamily("Arial");
#endif
    font.setPointSize(16);
    m_text->setDefaultFont(font);
    m_text->setPlainText(tr("insert text here..."));
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
    m_text->setHtml(htmlCode);
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
    if (m_textSize.width() < 1 || m_textSize.height() < 1)
        return AbstractContent::contentHeightForWidth(width);
    return (m_textSize.height() * width) / m_textSize.width();
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

    // skip if no photo

/*    painter->save();
    QRectF r = option->exposedRect;
    painter->translate(-dd->controlOffset());
    r.translate(dd->controlOffset());
*/
    //QTextDocumentLayout * layout = qobject_cast<QTextDocumentLayout *>(m_text->documentLayout());

    //m_text->documentLayout();

    // the layout might need to expand the root frame to
    // the viewport if NoWrap is set
    //if (layout)
    //    layout->setViewport(boundingRect().toRect());

    QRect rect = contentsRect();
    painter->save();
    painter->translate(-rect.width() / 2, -rect.height() / 2);
    if (m_textSize.width() > 0 && m_textSize.height() > 0) {
        qreal xScale = (qreal)rect.width() / (qreal)m_textSize.width();
        qreal yScale = (qreal)rect.height() / (qreal)m_textSize.height();
        if (!qFuzzyCompare(xScale, 1.0) || !qFuzzyCompare(yScale, 1.0))
            painter->scale(xScale, yScale);
    }
    m_text->drawContents(painter);

    //if (layout)
    //    layout->setViewport(QRect());

    painter->restore();
}

void TextContent::slotTextResized(const QSizeF & size)
{
    double prevXScale = 1.0;
    double prevYScale = 1.0;
    if (m_textSize.width() > 0 && m_textSize.height() > 0) {
        QSize cSize = contentsRect().size();
        prevXScale = (qreal)cSize.width() / (qreal)m_textSize.width();
        prevYScale = (qreal)cSize.height() / (qreal)m_textSize.height();
    }

    int w = size.width();
    int h = size.height();
    m_textSize = QSizeF(w, h);
    w = (int)(prevXScale * (qreal)w);
    h = (int)(prevYScale * (qreal)h);
    resizeContents(QRect(-w / 2, -h / 2, w, h));
}
