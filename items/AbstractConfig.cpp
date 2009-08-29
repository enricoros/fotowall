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

#include "AbstractConfig.h"
#include "AbstractContent.h"
#include "Desk.h"
#include "RenderOpts.h"
#include "StyledButtonItem.h"
#include "ui_AbstractConfig.h"
#include "frames/FrameFactory.h"
#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmapCache>
#include <QPushButton>
#include <QStyle>
#include <QWidget>
#include <QSettings>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

AbstractConfig::AbstractConfig(AbstractContent * content, QGraphicsItem * parent)
    : QGraphicsProxyWidget(parent)
    , m_content(content)
    , m_commonUi(new Ui::AbstractConfig())
    , m_closeButton(0)
    , m_okButton(0)
    , m_frame(FrameFactory::defaultPanelFrame())
{
    // close button
    m_closeButton = new StyledButtonItem(tr(" x "), font(), this);//this, ":/data/button-close.png", ":/data/button-close-hovered.png", ":/data/button-close-pressed.png");
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(slotRequestClose()));

    // WIDGET setup
    QWidget * widget = new QWidget();
#if QT_VERSION < 0x040500
    widget->setAttribute(Qt::WA_NoSystemBackground, true);
#else
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    m_commonUi->setupUi(widget);

    populateFrameList();

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
    connect(m_commonUi->newFrame, SIGNAL(clicked()), this, SLOT(on_newFrame_clicked()));
    connect(m_commonUi->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(on_listWidget_itemSelectionChanged()));
    connect(m_commonUi->reflection, SIGNAL(toggled(bool)), this, SLOT(on_reflection_toggled(bool)));

    // ITEM setup
    setWidget(widget);
    static qreal s_propZBase = 99999;
    setZValue(s_propZBase++);

#if QT_VERSION >= 0x040600
    // fade in animation
    QPropertyAnimation * ani = new QPropertyAnimation(this, "opacity");
    ani->setEasingCurve(QEasingCurve::OutCubic);
    ani->setDuration(400);
    ani->setStartValue(0.0);
    ani->setEndValue(1.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#endif
}

AbstractConfig::~AbstractConfig()
{
    delete m_frame;
    delete m_commonUi;
}

void AbstractConfig::dispose()
{
#if QT_VERSION >= 0x040600
    // fade out animation, then delete
    QPropertyAnimation * ani = new QPropertyAnimation(this, "opacity");
    connect(ani, SIGNAL(finished()), this, SLOT(deleteLater()));
    ani->setEasingCurve(QEasingCurve::OutCubic);
    ani->setDuration(200);
    ani->setEndValue(0.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    // delete this now
    deleteLater();
#endif
}

AbstractContent * AbstractConfig::content() const
{
    return m_content;
}

void AbstractConfig::keepInBoundaries(const QRect & rect)
{
    QRect r = mapToScene(boundingRect()).boundingRect().toRect();
    r.setLeft(qBound(rect.left(), r.left(), rect.right() - r.width()));
    r.setTop(qBound(rect.top(), r.top(), rect.bottom() - r.height()));
    setPos(r.topLeft());
}

void AbstractConfig::addTab(QWidget * widget, const QString & label, bool front, bool setCurrent)
{
    // insert on front/back
    int idx = m_commonUi->tab->insertTab(front ? 0 : m_commonUi->tab->count(), widget, label);

    // show if requested
    if (setCurrent)
        m_commonUi->tab->setCurrentIndex(idx);

    // adjust size after inserting the tab
    if (m_commonUi->tab->parentWidget())
        m_commonUi->tab->parentWidget()->adjustSize();
}

void AbstractConfig::showOkButton(bool show)
{
    if (show) {
        if (!m_okButton) {
            m_okButton = new StyledButtonItem(tr("ok"), font(), this);
            connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
            layoutButtons();
        }
        m_okButton->show();
    } else if (m_okButton)
        m_okButton->hide();
}

void AbstractConfig::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsProxyWidget::mousePressEvent(event);
    if (!event->isAccepted() && event->button() == Qt::RightButton)
        slotRequestClose();
    event->accept();
}

void AbstractConfig::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsProxyWidget::mouseDoubleClickEvent(event);
    event->accept();
}

void AbstractConfig::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // draw custom background
    painter->save();
    m_frame->paint(painter, boundingRect().toRect(), false, false);
    painter->restore();

    // unbreak parent
    QGraphicsProxyWidget::paint(painter, option, widget);
}

void AbstractConfig::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    layoutButtons();
    QGraphicsProxyWidget::resizeEvent(event);
}

void AbstractConfig::slotRequestClose()
{
    Desk * desk = static_cast<Desk *>(scene());
    desk->slotDeleteConfig(this);
}

void AbstractConfig::populateFrameList()
{
    m_commonUi->listWidget->clear();
    // add frame items to the listview
    foreach (quint32 frameClass, FrameFactory::classes()) {
        // make icon from frame preview
        Frame * frame = FrameFactory::createFrame(frameClass);
        QIcon icon(frame->preview(32, 32));
        delete frame;

        // add the item to the list (and attach it the class)
        QListWidgetItem * item = new QListWidgetItem(icon, QString(), m_commonUi->listWidget);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, frameClass);
    }
}

void AbstractConfig::layoutButtons()
{
    // layout the close button
    QRect cRect = boundingRect().toRect().adjusted(12, 12, -12, -12);
    if (QApplication::isLeftToRight()) {
        m_closeButton->setPos(cRect.right() - m_closeButton->boundingRect().width(), cRect.top());
        if (m_okButton)
            m_okButton->setPos(m_closeButton->pos().x() - m_okButton->boundingRect().width() - 8, cRect.top());
    } else {
        m_closeButton->setPos(cRect.left(), cRect.top());
        if (m_okButton)
            m_okButton->setPos(cRect.left() + m_closeButton->boundingRect().width() + 8, cRect.top());
    }
}

void AbstractConfig::on_newFrame_clicked()
{
    QStringList framesPath = QFileDialog::getOpenFileNames(0, tr("Choose frame images"), QString(), tr("Images (*.svg)"));
    if (!framesPath.isEmpty())
    foreach (QString frame, framesPath) {
        FrameFactory::addSvgFrame(frame);
    }
    populateFrameList();
}

void AbstractConfig::on_applyLooks_clicked()
{
    emit applyLook(m_content->frameClass(), m_content->mirrorEnabled(), true);
}

void AbstractConfig::on_listWidget_itemSelectionChanged()
{
    // get the frameClass
    QList<QListWidgetItem *> items = m_commonUi->listWidget->selectedItems();
    if (items.isEmpty())
        return;
    QListWidgetItem * item = items.first();
    quint32 frameClass = item->data(Qt::UserRole).toUInt();
    if (!frameClass)
        return;
    emit applyLook(frameClass, m_content->mirrorEnabled(), false);
}

void AbstractConfig::on_reflection_toggled(bool checked)
{
    RenderOpts::LastMirrorEnabled = checked;
    emit applyLook(m_content->frameClass(), checked, false);
}
