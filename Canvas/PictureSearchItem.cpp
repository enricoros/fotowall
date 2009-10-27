/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "PictureSearchItem.h"
#ifdef ENABLE_GCOMPLETION
#include "3rdparty/gsuggest.h"
#endif
#include "Shared/PictureServices/FlickrPictureService.h"
#include "Shared/PictureServices/GoogleImagesPictureService.h"
#include <QBasicTimer>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QTime>

#define FRAME_RADIUS 6

class SearchSymbol : public QWidget
{
    public:
        SearchSymbol(QWidget * parent)
          : QWidget(parent)
        {
            setFixedSize(25, 25);
            m_timer.start(50, this);
            m_time.start();
        }

    protected:
        void timerEvent(QTimerEvent * event)
        {
            if (event->timerId() != m_timer.timerId())
                return QWidget::timerEvent(event);
            update();
        }

        void paintEvent(QPaintEvent * /*event*/)
        {
            QPainter p(this);
            int elapsed = m_time.elapsed();
            if (elapsed < 1000)
                p.setOpacity((qreal)elapsed / 1000.0);
            int idx = 2 * elapsed - 90*16;
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::blue);
            p.drawPie(rect(), -idx, 50 * 16);
            p.drawPie(rect(), -idx + 180 * 16, 50 * 16);
        }

    private:
        QBasicTimer m_timer;
        QTime m_time;
};

class MyListWidget : public QListWidget
{
    public:
        MyListWidget(QWidget * parent)
          : QListWidget(parent)
          , m_pictureService(0)
        {
        }

        void setPictureService(AbstractPictureService * service)
        {
            m_pictureService = service;
        }

        void startDrag(Qt::DropActions supportedDropActions)
        {
            QList<QListWidgetItem *> items = selectedItems();
            int count = items.size();
            if (count < 1 || !m_pictureService)
                return;

            // make the drag pixmap and the indices data
            QStringList indices;
            QPixmap dragPixmap(50 + 10 * (count - 1), 50 + 10 * (count - 1));
            dragPixmap.fill(Qt::transparent);
            QPainter dragPainter(&dragPixmap);
            int i = 0;
            foreach (QListWidgetItem * item, items) {
                int idx = row(item);
                m_pictureService->startPrefetch(idx);
                dragPainter.drawPixmap(i * 10, i * 10, item->icon().pixmap(50, 50));
                indices.append(QString::number(idx));
                i++;
            }
            dragPainter.end();

            // create and execute the drag operation
            QDrag * drag = new QDrag(this);
            drag->setPixmap(dragPixmap);
            QMimeData * mimeData = new QMimeData();
            mimeData->setData("picturesearch/idx",indices.join(",").toLatin1());
            drag->setMimeData(mimeData);
            Qt::DropAction action = drag->exec(supportedDropActions, Qt::CopyAction);

            // abort prefetches if action was not accepted
            if (action != Qt::CopyAction)
                foreach (QListWidgetItem * item, items)
                    m_pictureService->stopPrefetch(row(item));
        }

    private:
        AbstractPictureService * m_pictureService;
};

class MyLineEdit : public QLineEdit
{
    public:
        MyLineEdit(QWidget * parent = 0)
          : QLineEdit(parent)
          , m_welcome(true)
        {
            // use a transparent look
            setFrame(false);
            QPalette pal;
            pal.setBrush(QPalette::Base, Qt::transparent);
            setPalette(pal);

            // inital text
            setText(tr("Type here..."));
            selectAll();
        }

        // ::QWidget
        void paintEvent(QPaintEvent * event)
        {
            // customize background
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing, false);
            //painter.setPen(Qt::NoPen);//QPen(Qt::lightGray, 1));
            //painter.setBrush(Qt::white);
            //painter.drawRect(QRectF(rect()).adjusted(0.5, 0.5, -1.5, -1.5));
            painter.setPen(QPen(Qt::darkGray, 1, Qt::DotLine));
            painter.drawLine(2, height() - 2, width() - 3, height() - 2);
            painter.end();

            // unbreak drawing
            QLineEdit::paintEvent(event);
        }
        void keyPressEvent(QKeyEvent * event)
        {
            normalMode();
            QLineEdit::keyPressEvent(event);
        }
        void mousePressEvent(QMouseEvent * event)
        {
            normalMode();
            QLineEdit::mousePressEvent(event);
        }

    private:
        void normalMode()
        {
            if (m_welcome) {
                m_welcome = false;
                clear();
            }
        }
        bool m_welcome;
};

// included here because it needs the definitions above
#include "ui_PictureSearchItem.h"

int PictureSearchItem::LastProvider = 0;

PictureSearchItem::PictureSearchItem(QNetworkAccessManager * extAccessManager, QGraphicsItem * parent)
    : QGraphicsProxyWidget(parent)
    , m_extAccessManager(extAccessManager)
    , m_pictureService(0)
#ifdef ENABLE_GCOMPLETION
    , m_completion(0)
#endif
    , m_ui(new Ui_PictureSearchItem())
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
    m_ui->listWidget->hide();
    QPalette pal;
    pal.setBrush(QPalette::Base, Qt::transparent);
    m_ui->listWidget->setPalette(pal);
    QFont font;
    font.setPointSize(font.pointSize() - 1);
    widget->setFont(font);
    connect(m_ui->searchButton, SIGNAL(clicked()), this, SLOT(slotSearchClicked()));
    connect(m_ui->lineEdit, SIGNAL(returnPressed()), m_ui->searchButton, SLOT(click()));
    connect(m_ui->fRadio, SIGNAL(toggled(bool)), this, SLOT(slotProviderChanged()));
    connect(m_ui->gRadio, SIGNAL(toggled(bool)), this, SLOT(slotProviderChanged()));
    if (LastProvider == 0)
        m_ui->fRadio->setChecked(true);
    else if (LastProvider == 1)
        m_ui->gRadio->setChecked(true);
    slotProviderChanged();

    // embed and layout widget
    setContentsMargins(0, 0, 0, 0);
    setWidget(widget);
    setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
    m_ui->lineEdit->setFocus();
    adjustSize();

    // init texts
    slotSearchEnded(false);

#ifdef ENABLE_GCOMPLETION
    // apply google completion to widget
    m_completion = new GSuggestCompletion(m_ui->lineEdit);
#endif
}

PictureSearchItem::~PictureSearchItem()
{
    if (m_ui->fRadio->isChecked())
        LastProvider = 0;
    else if (m_ui->gRadio->isChecked())
        LastProvider = 1;
    m_extAccessManager = 0;
#ifdef ENABLE_GCOMPLETION
    delete m_completion;
#endif
    delete m_searchSymbol;
    m_searchSymbol = 0;
    if (m_pictureService)
        m_pictureService->disconnect(0, 0, 0);
    delete m_pictureService;
    delete m_ui;
}

AbstractPictureService * PictureSearchItem::pictureService() const
{
    return m_pictureService;
}

void PictureSearchItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // draw background frame
    QLinearGradient lg(0, 0, 0, 50);
    if (m_ui->fRadio->isChecked())
        lg.setColorAt(0.0, QColor(255, 200, 200, 200));
    else if (m_ui->gRadio->isChecked())
        lg.setColorAt(0.0, QColor(200, 220, 255, 200));
    lg.setColorAt(1.0, QColor(200, 200, 200, 220));
    painter->setBrush(lg);
    painter->setPen(QPen(Qt::darkGray, 1));
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRectF boundaries = boundingRect().adjusted(0.5 - FRAME_RADIUS, 0.5 - FRAME_RADIUS, 0.5, 0.5);
    painter->drawRoundedRect(boundaries, FRAME_RADIUS, FRAME_RADIUS, Qt::AbsoluteSize);

    // speed up svg drawing and unbreak proxy widget
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, false);
    QGraphicsProxyWidget::paint(painter, option, widget);
}

void PictureSearchItem::slotProviderChanged()
{
    // no need to create the provider here, it will be created when searching
    m_ui->googleOptions->setVisible(m_ui->gRadio->isChecked());
    update();
}

void PictureSearchItem::slotSearchClicked()
{
    // search...
    if (!m_searchSymbol) {
        // get the current search term
#ifdef ENABLE_GCOMPLETION
        m_completion->preventSuggest();
#endif
        QString searchName = m_ui->lineEdit->text();
        if (searchName.isEmpty())
            return;

        // start a picture search
        if (!m_pictureService) {
            if (m_ui->fRadio->isChecked())
                m_pictureService = new FlickrPictureService("292287089cdba89fdbd9994830cc4327", m_extAccessManager, this);
            else if (m_ui->gRadio->isChecked()) {
                GoogleImagesPictureService * gis = new GoogleImagesPictureService(m_extAccessManager, this);
                gis->configure(m_ui->contentCombo->currentIndex(), m_ui->sizeCombo->currentIndex());
                m_pictureService = gis;
            } else {
                qWarning("PictureSearchItem::slotSearchClicked: unknown provider");
                return;
            }
            m_ui->fRadio->setVisible(false);
            m_ui->gRadio->setVisible(false);
            m_ui->googleOptions->setVisible(false);
            connect(m_pictureService, SIGNAL(searchStarted()), this, SLOT(slotSearchBegun()));
            connect(m_pictureService, SIGNAL(searchResult(int,QString,int,int)), this, SLOT(slotSearchResult(int,QString,int,int)));
            connect(m_pictureService, SIGNAL(searchThumbnail(int,QPixmap)), this, SLOT(slotSearchThumbnail(int,QPixmap)));
            connect(m_pictureService, SIGNAL(searchEnded(bool)), this, SLOT(slotSearchEnded(bool)));
            m_ui->listWidget->setPictureService(m_pictureService);
        }
        m_pictureService->searchPics(searchName);
    }

    // or cancel...
    else if (m_pictureService) {
        m_pictureService->dropSearch();
        m_ui->listWidget->clear();
    }
}

void PictureSearchItem::slotSearchBegun()
{
    m_ui->listWidget->clear();
    m_ui->listWidget->show();
    if (!m_searchSymbol) {
        m_searchSymbol = new SearchSymbol(m_ui->listWidget->viewport());
        m_searchSymbol->move(2, 2);
        m_searchSymbol->show();
    }
    m_ui->searchButton->setText(tr("Cancel"));
}

void PictureSearchItem::slotSearchResult(int idx, const QString & title, int thumb_w, int thumb_h)
{
    // create the placeholder Icon
    QLinearGradient lg(0, 0, 0, thumb_h);
    lg.setColorAt(0.0, Qt::lightGray);
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
#else
    Q_UNUSED(title);
#endif

    // add it the ListWidget
    m_ui->listWidget->insertItem(idx, item);
}

void PictureSearchItem::slotSearchThumbnail(int idx, const QPixmap & thumbnail)
{
    // update the pixmap of the Item
    if (QListWidgetItem * item = m_ui->listWidget->item(idx))
        item->setIcon(thumbnail);
}

void PictureSearchItem::slotSearchEnded(bool)
{
    if (m_searchSymbol) {
        delete m_searchSymbol;
        m_searchSymbol = 0;
    }
    m_ui->searchButton->setText(tr("Search"));
}
