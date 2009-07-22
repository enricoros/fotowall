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

#include "WebContentSelectorItem.h"
#include "FlickrInterface.h"
#include "frames/FrameFactory.h"
#include "frames/Frame.h"
#include "3rdparty/gsuggest.h"
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QListWidget>
#include <QPainter>

class MyListWidget : public QListWidget
{
    public:
        MyListWidget(QWidget * parent)
          : QListWidget(parent)
          , m_flickr(0)
        {
        }

        void setFlickrInterface(FlickrInterface * interface)
        {
            m_flickr = interface;
        }

        void startDrag(Qt::DropActions supportedDropActions)
        {
            QList<QListWidgetItem *> items = selectedItems();
            int count = items.size();
            if (count < 1 || !m_flickr)
                return;

            // make the drag pixmap and the indices data
            QStringList indices;
            QPixmap dragPixmap(50 + 10 * (count - 1), 50 + 10 * (count - 1));
            dragPixmap.fill(Qt::transparent);
            QPainter dragPainter(&dragPixmap);
            int i = 0;
            foreach (QListWidgetItem * item, items) {
                int idx = row(item);
                m_flickr->startPrefetch(idx);
                dragPainter.drawPixmap(i * 10, i * 10, item->icon().pixmap(50, 50));
                indices.append(QString::number(idx));
                i++;
            }
            dragPainter.end();

            // create and execute the drag operation
            QDrag * drag = new QDrag(this);
            drag->setPixmap(dragPixmap);
            QMimeData * mimeData = new QMimeData();
            mimeData->setData("webselector/idx",indices.join(",").toLatin1());
            drag->setMimeData(mimeData);
            Qt::DropAction action = drag->exec(supportedDropActions, Qt::CopyAction);

            // abort prefetches if action was not accepted
            if (action != Qt::CopyAction)
                foreach (QListWidgetItem * item, items)
                    m_flickr->stopPrefetch(row(item));
        }

    private:
        FlickrInterface * m_flickr;
};

#include "ui_WebContentSelectorItem.h"

WebContentSelectorItem::WebContentSelectorItem(QGraphicsItem * parent)
    : QGraphicsWidget(parent)
    , m_frame(FrameFactory::createFrame(0x1001 /*HARDCODED*/))
    , m_flickr(0)
    , m_completion(0)
    , m_ui(new Ui_WebContentSelectorItem())
    , m_searchSymbol(0)
{
    // create & customize Selector widget
    QWidget * widget = new QWidget();
#if QT_VERSION >= 0x040500
    widget->setAttribute(Qt::WA_TranslucentBackground);
#else
    widget->setAttribute(Qt::WA_NoSystemBackground, true);
#endif
    m_ui->setupUi(widget);
    QPalette pal;
    pal.setBrush(QPalette::Base, Qt::transparent);
    m_ui->listWidget->setPalette(pal);
    m_searchSymbol = new QLabel(tr("Loading..."), m_ui->listWidget->viewport());
    m_searchSymbol->move(2, 2);
    m_searchSymbol->hide();;
    connect(m_ui->searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

    // embed and layout widget
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
    QGraphicsLinearLayout * vLay = new QGraphicsLinearLayout(Qt::Vertical, this);
    vLay->addItem(proxy);
    setFlags(ItemIsSelectable | ItemIsFocusable | ItemAcceptsInputMethod);
    m_ui->lineEdit->setFocus();

    // apply google completion to widget
    m_completion = new GSuggestCompletion(m_ui->lineEdit);
}

WebContentSelectorItem::~WebContentSelectorItem()
{
    delete m_completion;
    delete m_flickr;
    delete m_frame;
    delete m_ui;
}

FlickrInterface * WebContentSelectorItem::flickrInterface() const
{
    return m_flickr;
}

void WebContentSelectorItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // draw frame
    m_frame->paint(painter, boundingRect().toRect(), false, false);
}

void WebContentSelectorItem::doSearch()
{
    // get the current search term
    m_completion->preventSuggest();
    QString searchName = m_ui->lineEdit->text();
    if (searchName.isEmpty())
        return;

    // start a flickr search
    if (!m_flickr) {
        m_flickr = new FlickrInterface(this);
        connect(m_flickr, SIGNAL(searchStarted()), this, SLOT(slotSearchBegun()));
        connect(m_flickr, SIGNAL(searchResult(int,QString,int,int)), this, SLOT(slotSearchResult(int,QString,int,int)));
        connect(m_flickr, SIGNAL(searchThumbnail(int,QPixmap)), this, SLOT(slotSearchThumbnail(int,QPixmap)));
        connect(m_flickr, SIGNAL(searchEnded()), this, SLOT(slotSearchEnded()));
        m_ui->listWidget->setFlickrInterface(m_flickr);
    }
    m_flickr->searchPics(searchName);
}

void WebContentSelectorItem::slotSearchBegun()
{
    m_ui->listWidget->clear();
    m_searchSymbol->show();
}

void WebContentSelectorItem::slotSearchResult(int idx, const QString & title, int thumb_w, int thumb_h)
{
    // create the placeholder Icon
    QLinearGradient lg(0, 0, 0, thumb_h);
    lg.setColorAt(1.0, Qt::lightGray);
    lg.setColorAt(1.0, Qt::darkGray);
    QPixmap placeHolderIcon(thumb_w, thumb_h);
    QPainter painter(&placeHolderIcon);
    painter.fillRect(0, 0, thumb_w, thumb_h, lg);
    painter.end();

    // create the ListWidget Item
    QListWidgetItem * item = new QListWidgetItem;
    item->setIcon(placeHolderIcon);
#if 0
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(title);
#endif

    // add it the ListWidget
    m_ui->listWidget->insertItem(idx, item);
}

void WebContentSelectorItem::slotSearchThumbnail(int idx, const QPixmap & thumbnail)
{
    // update the pixmap of the Item
    if (QListWidgetItem * item = m_ui->listWidget->item(idx))
        item->setIcon(thumbnail);
}

void WebContentSelectorItem::slotSearchEnded()
{
    m_searchSymbol->hide();
}
