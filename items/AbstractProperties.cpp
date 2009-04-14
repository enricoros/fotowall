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

#include "AbstractProperties.h"
#include "AbstractContent.h"
#include "RenderOpts.h"
#include "ui_AbstractProperties.h"
#include "frames/FrameFactory.h"
#include <QGraphicsSceneMouseEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmapCache>
#include <QPushButton>
#include <QStyle>
#include <QWidget>
#include <QSettings>

//BEGIN PixmapButton
class PixmapButton : public QGraphicsItem
{
    public:
        PixmapButton(QGraphicsItem * parent, const QString & normalPixmap, const QString & hoverPixmap, const QString & pressedPixmap = QString());

        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    private:
        bool m_hovered;
        bool m_pressed;
        QPixmap m_bNormal;
        QPixmap m_bHover;
        QPixmap m_bPress;
};

#define CACHED_LOAD(name, pixmap) \
    if (!QPixmapCache::find(name, pixmap)) { \
        if (pixmap.load(name)) \
            QPixmapCache::insert(name, pixmap); \
    }

PixmapButton::PixmapButton(QGraphicsItem * parent, const QString & normalPixmap, const QString & hoverPixmap, const QString & pressedPixmap)
    : QGraphicsItem(parent)
    , m_hovered(false)
    , m_pressed(false)
{
    // load the 3 pixmaps
    CACHED_LOAD(normalPixmap, m_bNormal);
    CACHED_LOAD(hoverPixmap, m_bHover);
    CACHED_LOAD(pressedPixmap, m_bPress);
    if (m_bHover.isNull())
        m_bHover = m_bNormal;
    if (m_bPress.isNull())
        m_bPress = m_bHover;

    // track mouse events
    setAcceptHoverEvents(true);
}

QRectF PixmapButton::boundingRect() const
{
    return m_bNormal.rect();
}

void PixmapButton::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
    if (m_pressed)
        painter->drawPixmap(option->rect, m_bPress, option->rect);
    else if (m_hovered)
        painter->drawPixmap(option->rect, m_bHover, option->rect);
    else
        painter->drawPixmap(option->rect, m_bNormal, option->rect);
}

void PixmapButton::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    m_hovered = true;
    update();
}

void PixmapButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    m_hovered = false;
    update();
}

void PixmapButton::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    m_pressed = true;
    update();
}

void PixmapButton::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    // check if going inside/outside
    bool inside = boundingRect().contains(event->pos());
    if (inside == m_pressed)
        return;
    m_pressed = inside;
    update();
}

void PixmapButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    update();

    // do click
    if (boundingRect().contains(event->pos())) {
        // HACK
        AbstractProperties * pp = dynamic_cast<AbstractProperties *>(parentItem());
        if (pp)
            pp->animateClose();
    }
}
//END PixmapButton


AbstractProperties::AbstractProperties(AbstractContent * content, QGraphicsItem * parent)
    : QGraphicsProxyWidget(parent)
    , m_content(content)
    , m_commonUi(new Ui::AbstractProperties())
    , m_closeButton(0)
    , m_frame(FrameFactory::defaultPanelFrame())
    , m_aniStep(0)
    , m_aniDirection(true)
{
    // close button
    m_closeButton = new PixmapButton(this, ":/data/button-close.png", ":/data/button-close-hovered.png", ":/data/button-close-pressed.png");

    // WIDGET setup
    QWidget * widget = new QWidget();
#if QT_VERSION < 0x040500
    widget->setAttribute(Qt::WA_NoSystemBackground, true);
#else
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    m_commonUi->setupUi(widget);

    // add frame items to the listview
    foreach (quint32 frameClass, FrameFactory::classes()) {
        // make icon from frame preview
        Frame * frame = FrameFactory::createFrame(frameClass);
        QIcon icon(frame->preview(48, 48));
        delete frame;

        // add the item to the list (and attach it the class)
        QListWidgetItem * item = new QListWidgetItem(icon, QString(), m_commonUi->listWidget);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, frameClass);
    }

    // select the frame
    quint32 frameClass = m_content->frameClass();
    if (frameClass != Frame::NoFrame) {
        for (int i = 0; i < m_commonUi->listWidget->count(); ++i) {
            QListWidgetItem * item = m_commonUi->listWidget->item(i);
            if (item->data(Qt::UserRole).toUInt() == frameClass) {
                item->setSelected(true);
                break;
            }
        }
    }

    // read other properties
    m_commonUi->reflection->setChecked(m_content->mirrorEnabled());

    connect(m_commonUi->front, SIGNAL(clicked()), m_content, SLOT(slotStackFront()));
    connect(m_commonUi->raise, SIGNAL(clicked()), m_content, SLOT(slotStackRaise()));
    connect(m_commonUi->lower, SIGNAL(clicked()), m_content, SLOT(slotStackLower()));
    connect(m_commonUi->back, SIGNAL(clicked()), m_content, SLOT(slotStackBack()));
    connect(m_commonUi->background, SIGNAL(clicked()), m_content, SIGNAL(backgroundMe()));
    connect(m_commonUi->save, SIGNAL(clicked()), m_content, SLOT(slotSaveAs()));
    connect(m_commonUi->del, SIGNAL(clicked()), m_content, SIGNAL(deleteItem()), Qt::QueuedConnection);
    // autoconnection doesn't work because we don't do ->setupUi(this), so here we connect manually
    connect(m_commonUi->applyLooks, SIGNAL(clicked()), this, SLOT(on_applyLooks_clicked()));
    connect(m_commonUi->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(on_listWidget_itemSelectionChanged()));
    connect(m_commonUi->reflection, SIGNAL(toggled(bool)), this, SLOT(on_reflection_toggled(bool)));

    // ITEM setup
    setWidget(widget);
    static qreal s_propZBase = 99999;
    setZValue(s_propZBase++);

    // Transition setup
    m_aniTimer.start(30, this);
}

AbstractProperties::~AbstractProperties()
{
    delete m_frame;
    delete m_commonUi;
}

AbstractContent * AbstractProperties::content() const
{
    return m_content;
}

void AbstractProperties::keepInBoundaries(const QRect & rect)
{
    QRect r = mapToScene(boundingRect()).boundingRect().toRect();
    r.setLeft(qBound(rect.left(), r.left(), rect.right() - r.width()));
    r.setTop(qBound(rect.top(), r.top(), rect.bottom() - r.height()));
    setPos(r.topLeft());
}

void AbstractProperties::animateClose()
{
    // closure animation
    m_aniDirection = false;
    m_aniTimer.start(20, this);
    emit closing();
}

void AbstractProperties::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsProxyWidget::mousePressEvent(event);
}

void AbstractProperties::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if (m_aniStep < 10)
        return;

    // draw custom background
#if QT_VERSION < 0x040500
    painter->fillRect(option->rect, QColor(250, 250, 250, 190));
#else
    m_frame->paint(painter, boundingRect().toRect(), false);
#endif

    // unbreak parent
#if 0
    if (m_aniStep >= 10 && m_aniStep <= 90)
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
#endif
    QGraphicsProxyWidget::paint(painter, option, widget);
}

void AbstractProperties::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    // layout the close button
    QRect cRect = boundingRect().toRect();
    if (m_frame)
        cRect = m_frame->contentsRect(cRect);
    if (QApplication::isLeftToRight())
        m_closeButton->setPos(cRect.right() - m_closeButton->boundingRect().width(), cRect.top());
    else
        m_closeButton->setPos(cRect.left(), cRect.top());

    // unbreak resize
    QGraphicsProxyWidget::resizeEvent(event);
}

void AbstractProperties::timerEvent(QTimerEvent * event)
{
    // only act on our events
    if (event->timerId() == m_aniTimer.timerId()) {
        if (m_aniDirection) {
            m_aniStep += 10;
            // end of FadeIn
            if (m_aniStep >= 100) {
                m_aniStep = 100;
                resetTransform();
                m_aniTimer.stop();
                return;
            }
            qreal xCenter = boundingRect().center().x();
            setTransform(QTransform().translate(xCenter, 0).rotate((90*(100-m_aniStep)*(100-m_aniStep)) / 10000, Qt::XAxis).translate(-xCenter, 0));
        } else {
            m_aniStep -= 10;
            // end of FadeOut
            if (m_aniStep <= 0) {
                m_aniStep = 0;
                resetTransform();
                m_aniTimer.stop();
                emit closed();
                return;
            }
            qreal xCenter = boundingRect().center().x();
            qreal yCenter = boundingRect().center().y();
            setTransform(QTransform().translate(xCenter, yCenter).rotate(-90 + (90*m_aniStep*m_aniStep) / 10000, Qt::XAxis).translate(-xCenter, -yCenter));
        }
    }
    QObject::timerEvent(event);
}

void AbstractProperties::addTab(QWidget * widget, const QString & label, bool front, bool select)
{
    // insert on front/back
    int idx = m_commonUi->tab->insertTab(front ? 0 : m_commonUi->tab->count(), widget, label);

    // select if requested
    if (select)
        m_commonUi->tab->setCurrentIndex(idx);

    // adjust size after inserting the tab
    if (m_commonUi->tab->parentWidget())
        m_commonUi->tab->parentWidget()->adjustSize();
}

void AbstractProperties::on_applyLooks_clicked()
{
    emit applyLooks(m_content->frameClass(), m_content->mirrorEnabled());
}

void AbstractProperties::on_listWidget_itemSelectionChanged()
{
    // get the frameClass
    QList<QListWidgetItem *> items = m_commonUi->listWidget->selectedItems();
    if (items.isEmpty())
        return;
    QListWidgetItem * item = items.first();
    quint32 frameClass = item->data(Qt::UserRole).toUInt();
    if (!frameClass)
        return;

    // create and set the frame
    Frame * frame = FrameFactory::createFrame(frameClass);
    if (frame)
        m_content->setFrame(frame);
    emit applyLookToSelection(m_content->frameClass(), m_content->mirrorEnabled());
}

void AbstractProperties::on_reflection_toggled(bool checked)
{
    RenderOpts::LastMirrorEnabled = checked;
    m_content->setMirrorEnabled(checked);
}
