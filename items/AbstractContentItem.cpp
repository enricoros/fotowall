/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "AbstractContentItem.h"
#include "ButtonItem.h"
#include "MirrorItem.h"
#include "RenderOpts.h"
#include "frames/FrameFactory.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <math.h>

AbstractContentItem::AbstractContentItem(QGraphicsScene * scene, QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_frame(0)
    , m_size(200, 150)
    , m_mirrorItem(0)
    , m_gfxChangeSignalTimer(0)
    , m_transformRefreshTimer(0)
    , m_transforming(false)
{
    // the buffered graphics changes timer
    m_gfxChangeSignalTimer = new QTimer(this);
    m_gfxChangeSignalTimer->setInterval(0);
    m_gfxChangeSignalTimer->setSingleShot(true);
    connect(m_gfxChangeSignalTimer, SIGNAL(timeout()), this, SIGNAL(gfxChange()));

    // customize item's behavior
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    // create child items
    m_scaleButton = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-scale.png"), this);
    connect(m_scaleButton, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotResize(const QPointF&,Qt::KeyboardModifiers)));
    connect(m_scaleButton, SIGNAL(doubleClicked()), this, SLOT(slotResetAspectRatio()));
    m_controlItems << m_scaleButton;

    m_rotateButton = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-rotate.png"), this);
    connect(m_rotateButton, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotRotate(const QPointF&)));
    connect(m_rotateButton, SIGNAL(doubleClicked()), this, SLOT(slotResetRotation()));
    m_controlItems << m_rotateButton;

    ButtonItem * bFront = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-order-front.png"), this);
    connect(bFront, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
    m_controlItems << bFront;

    ButtonItem * bConf = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-configure.png"), this);
    connect(bConf, SIGNAL(clicked()), this, SLOT(slotConfigure()));
    m_controlItems << bConf;

    ButtonItem * bDelete = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
    connect(bDelete, SIGNAL(clicked()), this, SIGNAL(deleteMe()));
    m_controlItems << bDelete;

    ButtonItem * bFlipH = new ButtonItem(ButtonItem::FlipH, Qt::blue, QIcon(":/data/action-flip-horizontal.png"), this);
    bFlipH->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    connect(bFlipH, SIGNAL(clicked()), this, SLOT(slotFlipHorizontally()));
    m_controlItems << bFlipH;

    ButtonItem * bFlipV = new ButtonItem(ButtonItem::FlipV, Qt::blue, QIcon(":/data/action-flip-vertical.png"), this);
    bFlipV->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    connect(bFlipV, SIGNAL(clicked()), this, SLOT(slotFlipVertically()));
    m_controlItems << bFlipV;

    // create default frame
    Frame * frame = FrameFactory::defaultPictureFrame();
    setFrame(frame);

    // hide and relayout buttons
    hoverLeaveEvent(0 /*HACK*/);
    geometryChanged();

    // add to the scene
    scene->addItem(this);

    // display mirror
    setMirrorEnabled(RenderOpts::LastMirrorEnabled);
}

AbstractContentItem::~AbstractContentItem()
{
    delete m_mirrorItem;
    delete m_frame;
}

void AbstractContentItem::setFrame(Frame * frame)
{
    delete m_frame;
    m_frame = frame;
    FrameFactory::setDefaultPictureClass(m_frame->frameClass());
    slotResetAspectRatio();
    geometryChanged();
    update();
    GFX_CHANGED();
}

quint32 AbstractContentItem::frameClass() const
{
    return m_frame->frameClass();
}

void AbstractContentItem::setMirrorEnabled(bool enabled)
{
    if (m_mirrorItem && !enabled) {
        m_mirrorItem->deleteLater();
        m_mirrorItem = 0;
    }
    if (enabled && !m_mirrorItem) {
        m_mirrorItem = new MirrorItem(this);
        connect(this, SIGNAL(gfxChange()), m_mirrorItem, SLOT(sourceUpdated()));
        connect(this, SIGNAL(destroyed()), m_mirrorItem, SLOT(deleteLater()));
    }
}

bool AbstractContentItem::mirrorEnabled() const
{
    return m_mirrorItem;
}

void AbstractContentItem::save(QDataStream & /*data*/) const
{
    qWarning("NI");
    /*
    data << m_size;
    data << pos();
    data << transform();
    data << zValue();
    data << m_fileName;
    */
}

bool AbstractContentItem::restore(QDataStream & /*data*/)
{
    qWarning("NI");
    /*
    prepareGeometryChange();
    data >> m_size;
    geometryChanged();
    QPointF p;
    data >> p;
    setPos(p);
    QTransform t;
    data >> t;
    setTransform(t);
    qreal zVal;
    data >> zVal;
    setZValue(zVal);
    QString fileName;
    data >> fileName;
    bool ok = loadPhoto(fileName);
    update();
    return ok;
    */
    return false;
}

void AbstractContentItem::ensureVisible(const QRectF & rect)
{
    // keep the center inside the scene rect
    QPointF center = pos();
    if (!rect.contains(center)) {
        center.setX(qBound(rect.left(), center.x(), rect.right()));
        center.setY(qBound(rect.top(), center.y(), rect.bottom()));
        setPos(center);
    }
}

bool AbstractContentItem::beingTransformed() const
{
    return m_transforming;
}

QRectF AbstractContentItem::boundingRect() const
{
    return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
}

void AbstractContentItem::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (ButtonItem * button, m_controlItems)
        button->show();
}

void AbstractContentItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (ButtonItem * button, m_controlItems)
        button->hide();
}

void AbstractContentItem::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void AbstractContentItem::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void AbstractContentItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // draw the Frame (background)
    if (!m_frame)
        return;
    QRect frameRect = boundingRect().toRect();
    m_frame->paint(painter, frameRect, false /*m_opaquePhoto FIXME */);
    ///if (!m_photo)
    ///    return;

    // use clip path for contents, if set
    if (m_frame->clipContents())
        painter->setClipPath(m_frame->contentsClipPath(frameRect));

    // blit if opaque picture (disabled for 4.5 too, since it relies too much on raster, i think)
#if QT_VERSION >= 0x040500
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_Source);
    /* Note: missing restore
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    */
#endif
}

void AbstractContentItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
        emit configureMe(event->scenePos().toPoint());
    QGraphicsItem::mousePressEvent(event);
}

void AbstractContentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_frame && m_frame->contentsRect(boundingRect().toRect()).contains(event->pos().toPoint()))
        emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void AbstractContentItem::wheelEvent(QGraphicsSceneWheelEvent * /*event*/)
{
}

void AbstractContentItem::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Delete)
        emit deleteMe();
    event->accept();
}

QVariant AbstractContentItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    // notify about graphics changes
    if (change == ItemTransformHasChanged ||
        change == ItemPositionHasChanged ||
        change == ItemVisibleHasChanged ||
        change == ItemEnabledHasChanged ||
        change == ItemSelectedHasChanged ||
#if QT_VERSION >= 0x040500
        change == ItemOpacityHasChanged ||
#endif
        change == ItemParentHasChanged ) {
        GFX_CHANGED();
    }

    // set mirror z level when it changes
    if (m_mirrorItem && change == ItemZValueHasChanged)
        m_mirrorItem->setZValue(zValue());
    if (m_mirrorItem && change == ItemVisibleHasChanged)
        m_mirrorItem->setVisible(isVisible());

    // keep the center inside the scene rect..
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        if (!rect.contains(newPos)) {
            newPos.setX(qBound(rect.left(), newPos.x(), rect.right()));
            newPos.setY(qBound(rect.top(), newPos.y(), rect.bottom()));
            return newPos;
        }
    }
    // ..or just apply the value
    return QGraphicsItem::itemChange(change, value);
}

QRect AbstractContentItem::contentsRect() const
{
    QRect cRect = boundingRect().toRect();
    if (!m_frame)
        return cRect;
    return m_frame->contentsRect(cRect);
}

void AbstractContentItem::GFX_CHANGED()
{
    if (m_gfxChangeSignalTimer)
        m_gfxChangeSignalTimer->start();
}

void AbstractContentItem::slotResize(const QPointF & controlPoint, Qt::KeyboardModifiers /*modifiers*/)
{
    QPoint newPos = mapFromScene(controlPoint).toPoint();
    QPoint oldPos = m_scaleButton->pos().toPoint();
    if (newPos == oldPos)
        return;

    // determine the new size
    QSize newSize((m_size.width() * newPos.x()) / oldPos.x(), (m_size.height() * newPos.y()) / oldPos.y());
// FIXME
///    if (modifiers != Qt::NoModifier && m_photo)
///        newSize.setHeight((m_photo->height() * m_size.width()) / m_photo->width());
    if (newSize.width() < 160)
        newSize.setWidth(160);
    if (newSize.height() < 90)
        newSize.setHeight(90);
    if (newSize == m_size)
        return;

    // change geometry
    m_transforming = true;
    prepareGeometryChange();
    m_size = newSize;
    geometryChanged();
    update();
    GFX_CHANGED();

    // start refresh timer
    if (!m_transformRefreshTimer) {
        m_transformRefreshTimer = new QTimer(this);
        connect(m_transformRefreshTimer, SIGNAL(timeout()), this, SLOT(slotResizeEnded()));
        m_transformRefreshTimer->setSingleShot(true);
    }
    m_transformRefreshTimer->start(400);
}

void AbstractContentItem::slotStackFront()
{
    emit changeStack(1);
}

void AbstractContentItem::slotStackRaise()
{
    emit changeStack(2);
}

void AbstractContentItem::slotStackLower()
{
    emit changeStack(3);
}

void AbstractContentItem::slotStackBack()
{
    emit changeStack(4);
}

void AbstractContentItem::slotSave()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Choose the file name"), QDir::current().path(), "PNG Image (*.png)");
    if (fileName.isNull())
        return;
    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
        fileName += ".png";

    // find out the Transform chain to mirror a rotated item
    QRectF sceneRectF = mapToScene(boundingRect()).boundingRect();
    QTransform tFromItem = transform() * QTransform(1, 0, 0, 1, pos().x(), pos().y());
    QTransform tFromPixmap = QTransform(1, 0, 0, 1, sceneRectF.left(), sceneRectF.top());
    QTransform tItemToPixmap = tFromItem * tFromPixmap.inverted();

    // render on the image
    int iHeight = (int)sceneRectF.height();
    if (m_mirrorItem)
        iHeight += (int)m_mirrorItem->boundingRect().height();
    QImage image((int)sceneRectF.width(), iHeight, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // enable hi-q rendering
    bool prevHQ = RenderOpts::HQRendering;
    RenderOpts::HQRendering = true;

    // draw the transformed item onto the pixmap
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setTransform(tItemToPixmap);
    paint(&p, 0, 0);
    if (m_mirrorItem) {
        p.resetTransform();
        p.translate(0, (qreal)((int)sceneRectF.height()));
        m_mirrorItem->paint(&p, 0, 0);
    }
    p.end();
    RenderOpts::HQRendering = prevHQ;

    // save image and check errors
    if (!image.save(fileName, "PNG") || !QFile::exists(fileName)) {
        QMessageBox::warning(0, tr("Picture Save Error"), tr("Error saving picture to the file %1").arg(fileName));
        return;
    }
}

void AbstractContentItem::slotConfigure()
{
    ButtonItem * item = dynamic_cast<ButtonItem *>(sender());
    if (!item)
        return;
    emit configureMe(item->scenePos().toPoint());
}

void AbstractContentItem::slotRotate(const QPointF & controlPoint)
{
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = m_rotateButton->pos();
    if (newPos == refPos)
        return;

    // set item rotation (set rotation relative to current)
    qreal refAngle = atan2(refPos.y(), refPos.x());
    qreal newAngle = atan2(newPos.y(), newPos.x());
    rotate(57.29577951308232 * (newAngle - refAngle)); // 180 * a / M_PI
}

void AbstractContentItem::slotResetAspectRatio()
{
    qWarning("NI - FIXME");
/*    // get the new size
    if (!m_frame)
        return;
    QSize newSize = m_frame->sizeForContentsRatio(m_size.width(), (qreal)m_photo->width() / (qreal)m_photo->height());
    if (!newSize.isValid() || newSize == m_size)
        return;

    // apply the new size
    prepareGeometryChange();
    m_size = newSize;
    geometryChanged();
    update();
    GFX_CHANGED();
    */
}

void AbstractContentItem::geometryChanged()
{
    if (!m_frame)
        return;
    QRect frameRect = boundingRect().toRect();

    // layout all buttons and text
    m_frame->layoutButtons(m_controlItems, frameRect);
    ///FIXME m_frame->layoutText(m_textItem, frameRect);
}

void AbstractContentItem::slotResetRotation()
{
    QTransform ident;
    setTransform(ident, false);
}

void AbstractContentItem::slotResizeEnded()
{
    m_transforming = false;
    update();
    GFX_CHANGED();
}
