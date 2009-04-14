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

#include "AbstractContent.h"
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
#include <QDebug>

AbstractContent::AbstractContent(QGraphicsScene * scene, QGraphicsItem * parent, bool noResize)
    : QGraphicsItem(parent)
    , m_rect(-100, -75, 200, 150)
    , m_frame(0)
    , m_frameTextItem(0)
    , m_scaleRatio(1.0)
    , m_transforming(false)
    , m_isSelected(false)
    , m_transformRefreshTimer(0)
    , m_gfxChangeTimer(0)
    , m_mirrorItem(0)
{
    // the buffered graphics changes timer
    m_gfxChangeTimer = new QTimer(this);
    m_gfxChangeTimer->setInterval(0);
    m_gfxChangeTimer->setSingleShot(true);
    connect(m_gfxChangeTimer, SIGNAL(timeout()), this, SIGNAL(gfxChange()));

    // customize item's behavior
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    // create child items
    if (!noResize) {
        ButtonItem * bScale = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-scale.png"), this);
        bScale->setToolTip(tr("Hold down SHIFT for ignoring aspect ratio.\nDouble click to restore the aspect ratio."));
        connect(bScale, SIGNAL(pressed()), this, SLOT(slotScaleStarted()));
        connect(bScale, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotScale(const QPointF&,Qt::KeyboardModifiers)));
        connect(bScale, SIGNAL(doubleClicked()), this, SLOT(slotResetRatio()));
        m_controlItems << bScale;
    }

    ButtonItem * bRotate = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-rotate.png"), this);
    bRotate->setToolTip(tr("Hold down SHIFT to snap the rotation.\nDouble click to align the object."));
    connect(bRotate, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotRotate(const QPointF&,Qt::KeyboardModifiers)));
    connect(bRotate, SIGNAL(doubleClicked()), this, SLOT(slotResetRotation()));
    m_controlItems << bRotate;

    //ButtonItem * bFront = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-order-front.png"), this);
    //bFront->setToolTip(tr("Raise"));
    //connect(bFront, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
    //m_controlItems << bFront;

    ButtonItem * bConf = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-configure.png"), this);
    bConf->setToolTip(tr("Change properties..."));
    connect(bConf, SIGNAL(clicked()), this, SLOT(slotConfigure()));
    m_controlItems << bConf;

    ButtonItem * bDelete = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
    bDelete->setToolTip(tr("Remove"));
    connect(bDelete, SIGNAL(clicked()), this, SIGNAL(deleteMe()));
    m_controlItems << bDelete;
 
    ButtonItem *bTransformXY = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
    bTransformXY->setToolTip(tr("Drag top or bottom to move along the X axis (perspective).\nDrag left or right to move along the Y axis.\nHold SHIFT to rotate faster.\nUse CTRL to cancel the transformations"));
    connect(bTransformXY, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotTransformXY(const QPointF&,Qt::KeyboardModifiers)));
    addButtonItem(bTransformXY);

    // create default frame
    Frame * frame = FrameFactory::defaultPictureFrame();
    setFrame(frame);

    // hide and layoutChildren buttons
    hoverLeaveEvent(0 /*HACK*/);
    layoutChildren();

    // add to the scene
    scene->addItem(this);

    // display mirror
    setMirrorEnabled(RenderOpts::LastMirrorEnabled);
}

AbstractContent::~AbstractContent()
{
    qDeleteAll(m_controlItems);
    delete m_mirrorItem;
    delete m_frameTextItem;
    delete m_frame;
}

void AbstractContent::setFrame(Frame * frame)
{
    delete m_frame;
    m_frame = frame;
    if (m_frame)
        FrameFactory::setDefaultPictureClass(m_frame->frameClass());
    adjustSize();
    layoutChildren();
    update();
    GFX_CHANGED();
}

quint32 AbstractContent::frameClass() const
{
    if (!m_frame)
        return Frame::NoFrame;
    return m_frame->frameClass();
}

#include <QGraphicsTextItem>
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

void AbstractContent::setFrameTextEnabled(bool enabled)
{
    // create the Text Item, if enabled...
    if (enabled && !m_frameTextItem) {
        m_frameTextItem = new MyTextItem(this);
        m_frameTextItem->setTextInteractionFlags(Qt::TextEditorInteraction);
        QFont f("Sans Serif");
        //f.setPointSizeF(7.5);
        m_frameTextItem->setFont(f);
        layoutChildren();
    }

    // ...or destroy it if disabled
    else if (!enabled && m_frameTextItem) {
        delete m_frameTextItem;
        m_frameTextItem = 0;
    }
}

bool AbstractContent::frameTextEnabled() const
{
    return m_frameTextItem;
}

void AbstractContent::setFrameText(const QString & text)
{
    if (!m_frameTextItem)
        return;
    m_frameTextItem->setPlainText(text);
}

QString AbstractContent::frameText() const
{
    if (!m_frameTextItem)
        return QString();
    return m_frameTextItem->toPlainText();
}

void AbstractContent::addButtonItem(ButtonItem * button)
{
    m_controlItems.append(button);
    layoutChildren();
}

void AbstractContent::setMirrorEnabled(bool enabled)
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

bool AbstractContent::mirrorEnabled() const
{
    return m_mirrorItem;
}

void AbstractContent::setSelected(bool state)
{ 
    m_isSelected = state;
    m_mirrorItem->sourceUpdated();
    update();
}

void AbstractContent::resize(const QRectF & rect)
{
    if (!rect.isValid() || rect == m_rect)
        return;
    prepareGeometryChange();
    m_rect = rect;
    layoutChildren();
    update();
    GFX_CHANGED();
}

void AbstractContent::adjustSize()
{
    // get contents 'ratio'
    int hfw = contentHeightForWidth(m_rect.width());
    if (hfw < 1)
        return;

    // compute the new rect
    QRectF newRect;
    if (m_frame) {
        double ratio = m_rect.width() / (double)hfw;
        QSize s = m_frame->sizeForContentsRatio(m_rect.width(), ratio);
        newRect = QRectF(-s.width() / 2, -s.height() / 2, s.width(), s.height());
    } else
        newRect = QRectF(m_rect.left(), -hfw / 2, m_rect.width(), hfw);

    // apply the new size
    resize(newRect);
}

void AbstractContent::ensureVisible(const QRectF & rect)
{
    // keep the center inside the scene rect
    QPointF center = pos();
    if (!rect.contains(center)) {
        center.setX(qBound(rect.left(), center.x(), rect.right()));
        center.setY(qBound(rect.top(), center.y(), rect.bottom()));
        setPos(center);
    }
}

bool AbstractContent::beingTransformed() const
{
    return m_transforming;
}

void AbstractContent::save(QDataStream & data) const
{
    data << m_rect;
    data << pos();
    data << transform();
    data << zValue();
    data << isVisible();
    bool hasText = frameTextEnabled();
    data << hasText;
    if (hasText)
        data << frameText();
    quint32 frameClass = m_frame ? m_frame->frameClass() : 0;
    data << frameClass;
}

bool AbstractContent::restore(QDataStream & data)
{
    prepareGeometryChange();
    data >> m_rect;
    layoutChildren();
    QPointF p;
    data >> p;
    setPos(p);
    QTransform t;
    data >> t;
    setTransform(t);
    qreal zVal;
    data >> zVal;
    setZValue(zVal);
    bool visible;
    data >> visible;
    setVisible(visible);
    bool hasText;
    data >> hasText;
    setFrameTextEnabled(hasText);
    if (hasText) {
        QString text;
        data >> text;
        setFrameText(text);
    }
    quint32 frameClass;
    data >> frameClass;
    setFrame(frameClass ? FrameFactory::createFrame(frameClass) : 0);
    update();
    return true;
}

QPixmap AbstractContent::renderAsBackground(const QSize & size) const
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    return pix;
}

AbstractProperties * AbstractContent::createProperties() const
{
    return 0;
}

QRectF AbstractContent::boundingRect() const
{
    return m_rect;
}

void AbstractContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (!m_frame)
        return;

    // draw the Frame
    bool opaqueContent = contentOpaque();
    QRect frameRect = m_rect.toRect();
    m_frame->paint(painter, frameRect, opaqueContent);

    if(m_isSelected) {
        painter->save();
        QBrush brush(Qt::blue, Qt::Dense4Pattern);
        QPainterPath path = m_frame->frameShape(frameRect);
        painter->setBrush(brush);
        painter->drawPath(path);
        painter->restore();
    }

    // use clip path for contents, if set
    if (m_frame->clipContents())
        painter->setClipPath(m_frame->contentsClipPath(frameRect));
}

QRect AbstractContent::contentsRect() const
{
    if (!m_frame)
        return m_rect.toRect();

    return m_frame->contentsRect(m_rect.toRect());
}

void AbstractContent::GFX_CHANGED() const
{
    if (m_gfxChangeTimer)
        m_gfxChangeTimer->start();
}

int AbstractContent::contentHeightForWidth(int /*width*/) const
{
    return -1;
}

bool AbstractContent::contentOpaque() const
{
    return false;
}

void AbstractContent::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (ButtonItem * button, m_controlItems)
        button->show();
}

void AbstractContent::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (ButtonItem * button, m_controlItems)
        button->hide();
}

void AbstractContent::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void AbstractContent::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void AbstractContent::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
        emit configureMe(event->scenePos().toPoint());
    else if(event->button() == Qt::LeftButton) {
        m_isSelected = true;
        update();
        if(event->modifiers() == Qt::ControlModifier) {
            emit addItemToSelection(this);
        } else {
            emit itemSelected(this);
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

void AbstractContent::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Delete)
        emit deleteMe();
    event->accept();
}

QVariant AbstractContent::itemChange(GraphicsItemChange change, const QVariant & value)
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

void AbstractContent::slotConfigure()
{
    ButtonItem * item = dynamic_cast<ButtonItem *>(sender());
    if (!item)
        return;
    emit configureMe(item->scenePos().toPoint());
}

void AbstractContent::slotStackFront()
{
    emit changeStack(1);
}

void AbstractContent::slotStackRaise()
{
    emit changeStack(2);
}

void AbstractContent::slotStackLower()
{
    emit changeStack(3);
}

void AbstractContent::slotStackBack()
{
    emit changeStack(4);
}

void AbstractContent::slotSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Choose the Image file"), QString(), tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if (fileName.isNull())
        return;
    if (QFileInfo(fileName).suffix().isEmpty())
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
    if (!image.save(fileName) || !QFile::exists(fileName)) {
        QMessageBox::warning(0, tr("File Error"), tr("Error saving the Object to '%1'").arg(fileName));
        return;
    }
}

void AbstractContent::layoutChildren()
{
    // layout buttons even if no frame
    if (!m_frame) {
        const int margin = 4;
        const int spacing = 4;
        int right = m_rect.right() - margin;
        int bottom = m_rect.bottom() + margin; // if no frame, offset the buttons a little on bottom
        foreach (ButtonItem * button, m_controlItems) {
            button->setPos(right - button->width() / 2, bottom - button->height() / 2);
            right -= button->width() + spacing;
        }
        return;
    }

    // layout all controls
    QRect frameRect = m_rect.toRect();
    m_frame->layoutButtons(m_controlItems, frameRect);

    // layout text, if present
    if (m_frameTextItem)
        m_frame->layoutText(m_frameTextItem, frameRect);
}

void AbstractContent::slotScaleStarted()
{
    if (m_rect.height() > 0)
        m_scaleRatio = (float)m_rect.width() / (float)m_rect.height();
}

void AbstractContent::slotScale(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers)
{
    ButtonItem * button = static_cast<ButtonItem *>(sender());
    QPoint newPos = mapFromScene(controlPoint).toPoint();
    QPoint oldPos = button->pos().toPoint();
    if (newPos == oldPos)
        return;

    // determine the new size
    int newWidth = (m_rect.width() * newPos.x()) / oldPos.x();
    int newHeight = (m_rect.height() * newPos.y()) / oldPos.y();
    // preserve aspect ratio if no modifiers are pressed
    if (modifiers == Qt::NoModifier) {
        float ratio = (float)newWidth / (float)newHeight;
        if (ratio > m_scaleRatio)
            newHeight = newWidth / m_scaleRatio;
        else
            newWidth = newHeight * m_scaleRatio;
    }
    if (newWidth < 50)
        newWidth = 50;
    if (newHeight < 40)
        newHeight = 40;
    if (newWidth == (int)m_rect.width() && newHeight == (int)m_rect.height())
        return;

    // change geometry
    m_transforming = true;
    resize(QRectF(-newWidth / 2, -newHeight / 2, newWidth, newHeight));

    // start refresh timer
    if (!m_transformRefreshTimer) {
        m_transformRefreshTimer = new QTimer(this);
        connect(m_transformRefreshTimer, SIGNAL(timeout()), this, SLOT(slotTransformEnded()));
        m_transformRefreshTimer->setSingleShot(true);
    }
    m_transformRefreshTimer->start(400);
}

void AbstractContent::slotRotate(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers)
{
    ButtonItem * button = static_cast<ButtonItem *>(sender());
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = button->pos();
    if (newPos == refPos)
        return;

    // set item rotation (set rotation relative to current)
    qreal refAngle = atan2(refPos.y(), refPos.x());
    qreal newAngle = atan2(newPos.y(), newPos.x());
    rotate(57.29577951308232 * (newAngle - refAngle)); // 180 * a / M_PI

    // snap to M_PI/4
    if (modifiers != Qt::NoModifier) {
        QTransform t = transform();
        QPointF ax = t.map(QPointF(1, 0));
        qreal rotAngle = atan2(ax.y(), ax.x());
        int fracts = (int)((rotAngle - 0.19635) / 0.39270);
        rotAngle = (qreal)fracts * 0.39270;
        setTransform(QTransform().rotateRadians(rotAngle));
    }
}

void AbstractContent::slotResetRatio()
{
    adjustSize();
}

void AbstractContent::slotResetRotation()
{
    QTransform ident;
    setTransform(ident, false);
}

void AbstractContent::slotTransformEnded()
{
    m_transforming = false;
    update();
    GFX_CHANGED();
}

void AbstractContent::slotTransformXY(const QPointF& controlPoint,Qt::KeyboardModifiers modifiers)
{
    static int angleX = 0;
    static int angleY = 0;
    ButtonItem * button = static_cast<ButtonItem *>(sender());
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = button->pos();
    if (newPos == refPos)
        return;

    int march=0;
    if(modifiers == Qt::NoModifier) march = 2;
    else if( modifiers == Qt::ControlModifier ) {
        angleX=0;
        angleY=0;
    }
    else march = 4;

    //// Perspective : move along X axis
    if(newPos.y() - refPos.y() > 70) angleX -= march;
    else if(newPos.y() - refPos.y() < -70) angleX += march;
    // Prevents the user from rotating too much (if so, buttons become unavailable).
    if (angleX > 80) angleX = 80;
    if (angleX < -80 ) angleX = -80;

    /// Move along Y axis
    if(newPos.x() - refPos.x() > 70) angleY -= march;
    else if(newPos.x() - refPos.x() < -70) angleY += march;
    if (angleY > 80) angleY = 80;
    if (angleY < -80 ) angleY = -80;
    setTransform(QTransform().rotate(angleX, Qt::XAxis).rotate(angleY, Qt::YAxis)); 
}

