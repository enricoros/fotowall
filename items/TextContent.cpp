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
    : AbstractContent(scene, parent, true)
    , m_text(0)
{
    setFrame(0);
    setFrameTextEnabled(false);
    setToolTip(tr("Right click to Edit the text"));

    // create a text document
    m_text = new QTextDocument(this);
    QAbstractTextDocumentLayout * layout = m_text->documentLayout();
    connect(layout, SIGNAL(documentSizeChanged(const QSizeF &)), this, SLOT(slotResize(const QSizeF &)));

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
    AbstractContent::fromXml(pe);

    // load text properties
    QString text = pe.firstChildElement("html-text").text();
    setHtml(text);
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
    m_text->drawContents(painter);
    //if (layout)
    //    layout->setViewport(QRect());

    painter->restore();
}

void TextContent::slotResize(const QSizeF & size)
{
    int w = size.width() + 4;
    int h = size.height() + 4;
    resizeContents(QRect(-w / 2, -h / 2, w, h));
}
