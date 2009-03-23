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

#include "PictureItem.h"
#include "ButtonItem.h"
#include "MirrorItem.h"
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

// from FotoWall.cpp
extern bool globalExportingFlag;

// init statics
bool PictureItem::s_defaultMirrorEnabled = true;

class MyTextItem : public QGraphicsTextItem {
    public:
        MyTextItem(QGraphicsItem * parent = 0)
            : QGraphicsTextItem(parent)
        {
        }

        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
            painter->save();
            painter->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform );
            QGraphicsTextItem::paint(painter, option, widget);
            painter->restore();
        }
};

#define GFX_CHANGED \
    if (m_gfxChangeSignalTimer) \
        m_gfxChangeSignalTimer->start();

PictureItem::PictureItem(QGraphicsScene * scene, QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_frame(0)
    , m_photo(0)
    , m_opaquePhoto(false)
    , m_size(200, 150)
    , m_mirrorItem(0)
    , m_gfxChangeSignalTimer(0)
    , m_scaleRefreshTimer(0)
    , m_scaling(false)
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

    m_textItem = new MyTextItem(this);
    m_textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont f("Sans Serif");
    //f.setPointSizeF(7.5);
    m_textItem->setFont(f);
    m_textItem->setPlainText(tr("..."));

    // create default frame
    Frame * frame = FrameFactory::defaultPictureFrame();
    setFrame(frame);

    // hide and relayout buttons
    hoverLeaveEvent(0 /*HACK*/);
    relayoutContents();

    // add to the scene
    scene->addItem(this);

    // display mirror
    setMirrorEnabled(s_defaultMirrorEnabled);
}

PictureItem::~PictureItem()
{
    delete m_mirrorItem;
    delete m_frame;
    delete m_photo;
}

bool PictureItem::loadPhoto(const QString & fileName, bool keepRatio, bool setName)
{
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = new CPixmap(fileName);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        m_fileName = QString();
        return false;
    }
    m_opaquePhoto = !m_photo->hasAlpha();
    m_fileName = fileName;
    if (keepRatio)
        slotResetAspectRatio();
    if (setName) {
        QString string = QFileInfo(fileName).fileName().section('.', 0, 0);
        string = string.mid(0, 10);
        m_textItem->setPlainText(string + " ...");
    }
    update();
    GFX_CHANGED
    return true;
}

QPixmap PictureItem::renderPhoto(const QSize & size) const
{
    if (m_photo)
        return m_photo->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return QPixmap();
}

void PictureItem::setFrame(Frame * frame)
{
    delete m_frame;
    m_frame = frame;
    FrameFactory::setDefaultPictureClass(m_frame->frameClass());
    slotResetAspectRatio();
    relayoutContents();
    update();
    GFX_CHANGED
}

quint32 PictureItem::frameClass() const
{
    return m_frame->frameClass();
}

bool PictureItem::mirrorEnabled() const
{
    return m_mirrorItem;
}

void PictureItem::setMirrorEnabled(bool enabled)
{
    if (m_mirrorItem && !enabled) {
        m_mirrorItem->deleteLater();
        m_mirrorItem = 0;
    }
    if (enabled && !m_mirrorItem) {
        m_mirrorItem = new MirrorItem(this);
        m_mirrorItem->show();
    }
}

void PictureItem::setEffect(int effectClass)
{
    // apply effect to pixmap
    switch (effectClass) {
        case 0:
            m_photo->invertColors();
            break;
        case 1:
            m_photo->toNVG();
            break;
        case 2:
            m_photo->toBlackAndWhite();
            break;
        default:
            qWarning("PictureItem::setEffect: effect %d is not implemented", effectClass);
            return;
    }

    // invalidate cache and repaint
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED
}

void PictureItem::save(QDataStream & data) const
{
    data << m_size;
    data << pos();
    data << transform();
    data << zValue();
    data << m_fileName;
    data << m_textItem->toPlainText();
}

bool PictureItem::restore(QDataStream & data)
{
    prepareGeometryChange();
    data >> m_size;
    relayoutContents();
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
    QString text;
    data >> text;
    m_textItem->setPlainText(text);
    update();
    return ok;
}

void PictureItem::ensureVisible(const QRectF & rect)
{
    // keep the center inside the scene rect
    QPointF center = pos();
    if (!rect.contains(center)) {
        center.setX(qBound(rect.left(), center.x(), rect.right()));
        center.setY(qBound(rect.top(), center.y(), rect.bottom()));
        setPos(center);
    }
}

QRectF PictureItem::boundingRect() const
{
    return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
}

void PictureItem::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (ButtonItem * button, m_controlItems)
        button->show();
}

void PictureItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (ButtonItem * button, m_controlItems)
        button->hide();
}

void PictureItem::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void PictureItem::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // load the first valid picture
    foreach (QUrl url, event->mimeData()->urls()) {
        if (loadPhoto(url.toLocalFile(), true, false)) {
            event->accept();
            return;
        }
    }
}

void PictureItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // draw the Frame (background)
    if (!m_frame)
        return;
    QRect frameRect = boundingRect().toRect();
    m_frame->paint(painter, frameRect, m_opaquePhoto);
    if (!m_photo)
        return;

    // use clip path for contents, if set
    if (m_frame->clipContents())
        painter->setClipPath(m_frame->contentsClipPath(frameRect));

    // blit if opaque picture (disabled for 4.5 too, since it relies too much on raster, i think)
#if QT_VERSION >= 0x040500
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

    // draw high-resolution photo when exporting png
    QRect targetRect = m_frame->contentsRect(frameRect);
    if (globalExportingFlag) {
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect, *m_photo);
        return;
    }

    // draw photo using caching and deferred rescales
    if (m_scaling) {
        if (!m_cachedPhoto.isNull())
            painter->drawPixmap(targetRect, m_cachedPhoto);
    } else {
        if (m_cachedPhoto.isNull() || m_cachedPhoto.size() != targetRect.size())
            m_cachedPhoto = m_photo->scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect.topLeft(), m_cachedPhoto);
    }

    // restore status
#if QT_VERSION >= 0x040500
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

void PictureItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
        emit configureMe(event->scenePos().toPoint());
    QGraphicsItem::mousePressEvent(event);
}

void PictureItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_frame && m_frame->contentsRect(boundingRect().toRect()).contains(event->pos().toPoint()))
        emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void PictureItem::wheelEvent(QGraphicsSceneWheelEvent * /*event*/)
{
}

void PictureItem::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Delete)
        emit deleteMe();
    event->accept();
}

QVariant PictureItem::itemChange(GraphicsItemChange change, const QVariant & value)
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
        GFX_CHANGED
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

void PictureItem::relayoutContents()
{
    if (!m_frame)
        return;
    QRect frameRect = boundingRect().toRect();

    // layout all buttons and text
    m_frame->layoutButtons(m_controlItems, frameRect);
    m_frame->layoutText(m_textItem, frameRect);
}

void PictureItem::slotResize(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers)
{
    QPoint newPos = mapFromScene(controlPoint).toPoint();
    QPoint oldPos = m_scaleButton->pos().toPoint();
    if (newPos == oldPos)
        return;

    // determine the new size
    QSize newSize((m_size.width() * newPos.x()) / oldPos.x(), (m_size.height() * newPos.y()) / oldPos.y());
    if (modifiers != Qt::NoModifier && m_photo)
        newSize.setHeight((m_photo->height() * m_size.width()) / m_photo->width());
    if (newSize.width() < 160)
        newSize.setWidth(160);
    if (newSize.height() < 90)
        newSize.setHeight(90);
    if (newSize == m_size)
        return;

    // change geometry
    m_scaling = true;
    prepareGeometryChange();
    m_size = newSize;
    relayoutContents();
    update();
    GFX_CHANGED

    // start refresh timer
    if (!m_scaleRefreshTimer) {
        m_scaleRefreshTimer = new QTimer(this);
        connect(m_scaleRefreshTimer, SIGNAL(timeout()), this, SLOT(slotResizeEnded()));
        m_scaleRefreshTimer->setSingleShot(true);
    }
    m_scaleRefreshTimer->start(400);
}

void PictureItem::slotFlipHorizontally()
{
    // delete the Photo and and recreate an H-mirrored one
    CPixmap * oldPhoto = m_photo;
    m_photo = new CPixmap(CPixmap::fromImage(oldPhoto->toImage().mirrored(true, false)));
    delete oldPhoto;
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED
}

void PictureItem::slotFlipVertically()
{
    // delete the Photo and and recreate a V-mirrored one
    CPixmap * oldPhoto = m_photo;
    m_photo = new CPixmap(CPixmap::fromImage(oldPhoto->toImage().mirrored(false, true)));
    delete oldPhoto;
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED
}

void PictureItem::slotStackFront()
{
    emit changeStack(1);
}

void PictureItem::slotStackRaise()
{
    emit changeStack(2);
}

void PictureItem::slotStackLower()
{
    emit changeStack(3);
}

void PictureItem::slotStackBack()
{
    emit changeStack(4);
}

void PictureItem::slotSave()
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
    bool prevFlag = globalExportingFlag;
    globalExportingFlag = true;

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
    globalExportingFlag = prevFlag;

    // save image and check errors
    if (!image.save(fileName, "PNG") || !QFile::exists(fileName)) {
        QMessageBox::warning(0, tr("Picture Save Error"), tr("Error saving picture to the file %1").arg(fileName));
        return;
    }
}

void PictureItem::slotConfigure()
{
    ButtonItem * item = dynamic_cast<ButtonItem *>(sender());
    if (!item)
        return;
    emit configureMe(item->scenePos().toPoint());
}

void PictureItem::slotRotate(const QPointF & controlPoint)
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

void PictureItem::slotResetAspectRatio()
{
    // get the new size
    if (!m_photo || m_photo->isNull() || !m_frame)
        return;
    QSize newSize = m_frame->sizeForContentsRatio(m_size.width(), (qreal)m_photo->width() / (qreal)m_photo->height());
    if (newSize == m_size)
        return;

    // apply the new size
    prepareGeometryChange();
    m_size = newSize;
    relayoutContents();
    update();
    GFX_CHANGED
}

void PictureItem::slotResetRotation()
{
    QTransform ident;
    setTransform(ident, false);
}

void PictureItem::slotResizeEnded()
{
    m_scaling = false;
    update();
    GFX_CHANGED
}
