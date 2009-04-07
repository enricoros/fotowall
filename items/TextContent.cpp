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

    // create a text document
    m_text = new QTextDocument(this);
    QAbstractTextDocumentLayout * layout = m_text->documentLayout();
    connect(layout, SIGNAL(documentSizeChanged(const QSizeF &)), this, SLOT(slotResize(const QSizeF &)));

    // template text
    m_text->setHtml(tr("<body><pre style='font-size: 12pt; color: #800;'>Insert text here</pre></body>"));
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

void TextContent::save(QDataStream & data) const
{
    AbstractContent::save(data);
}

bool TextContent::restore(QDataStream & data)
{
    return AbstractContent::restore(data);
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

    QRect rect = boundingRect().toRect();
    painter->save();
    painter->translate(-rect.width() / 2, -rect.height() / 2);
    m_text->drawContents(painter);
    //if (layout)
    //    layout->setViewport(QRect());

    painter->restore();

    if (option) {
        //if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus))
        //    qt_graphicsItem_highlightSelected(this, painter, option);
    }
}

void TextContent::slotResize(const QSizeF & size)
{
    int w = size.width() + 4;
    int h = size.height() + 4;
    resize(QRectF(-w / 2, -h / 2, w, h));
}
