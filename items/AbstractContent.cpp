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
#include "CornerItem.h"
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

AbstractContent::AbstractContent(QGraphicsScene * scene, QGraphicsItem * parent, bool noRescale)
    : QGraphicsItem(parent)
    , m_contentsRect(-100, -75, 200, 150)
    , m_frame(0)
    , m_frameTextItem(0)
    , m_dirtyTransforming(false)
    , m_transformRefreshTimer(0)
    , m_gfxChangeTimer(0)
    , m_mirrorItem(0)
    , m_xRotationAngle(0), m_yRotationAngle(0), m_zRotationAngle(0)
{
    // the buffered graphics changes timer
    m_gfxChangeTimer = new QTimer(this);
    m_gfxChangeTimer->setInterval(0);
    m_gfxChangeTimer->setSingleShot(true);
    connect(m_gfxChangeTimer, SIGNAL(timeout()), this, SIGNAL(gfxChange()));

    // customize item's behavior
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    // create child controls
    createCorner(Qt::TopLeftCorner, noRescale);
    createCorner(Qt::TopRightCorner, noRescale);
    createCorner(Qt::BottomLeftCorner, noRescale);
    createCorner(Qt::BottomRightCorner, noRescale);

    //ButtonItem * bFront = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-order-front.png"), this);
    //bFront->setToolTip(tr("Raise"));
    //connect(bFront, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
    //m_controlItems << bFront;

    ButtonItem * bConf = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-configure.png"), this);
    bConf->setToolTip(tr("Change properties..."));
    connect(bConf, SIGNAL(clicked()), this, SLOT(slotConfigure()));
    m_controlItems << bConf;

    ButtonItem *bTransformXY = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
    bTransformXY->setToolTip(tr("Drag top or bottom to move along the X axis (perspective).\nDrag left or right to move along the Y axis.\nHold SHIFT to rotate faster.\nUse CTRL to cancel the transformations"));
    connect(bTransformXY, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotPerspective(const QPointF&,Qt::KeyboardModifiers)));
    addButtonItem(bTransformXY);

    ButtonItem * bDelete = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
    bDelete->setToolTip(tr("Remove"));
    connect(bDelete, SIGNAL(clicked()), this, SIGNAL(deleteItem()));
    m_controlItems << bDelete;

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

QRect AbstractContent::contentsRect() const
{
    return m_contentsRect;
}

void AbstractContent::resizeContents(const QRect & rect, bool keepRatio)
{
    if (!rect.isValid())
        return;

    prepareGeometryChange();

    m_contentsRect = rect;
    if (keepRatio) {
        int hfw = contentHeightForWidth(rect.width());
        if (hfw > 1) {
            m_contentsRect.setTop(-hfw / 2);
            m_contentsRect.setHeight(hfw);
        }
    }

    if (m_frame)
        m_frameRect = m_frame->frameRect(m_contentsRect);
    else
        m_frameRect = m_contentsRect;

    layoutChildren();
    update();
    GFX_CHANGED();
}

void AbstractContent::resetContentsRatio()
{
    resizeContents(m_contentsRect, true);
}

void AbstractContent::delayedDirty(int ms)
{
    // tell rendering that we're changing stuff
    m_dirtyTransforming = true;

    // start refresh timer
    if (!m_transformRefreshTimer) {
        m_transformRefreshTimer = new QTimer(this);
        connect(m_transformRefreshTimer, SIGNAL(timeout()), this, SLOT(slotDirtyEnded()));
        m_transformRefreshTimer->setSingleShot(true);
    }
    m_transformRefreshTimer->start(ms);
}

void AbstractContent::setFrame(Frame * frame)
{
    delete m_frame;
    m_frame = frame;
    if (m_frame)
        FrameFactory::setDefaultPictureClass(m_frame->frameClass());
    resizeContents(m_contentsRect);
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

void AbstractContent::setRotation(double pan, double tilt, double roll)
{
    m_xRotationAngle = tilt;
    m_yRotationAngle = roll;
    m_zRotationAngle = pan;
    setTransform(QTransform().rotate(m_zRotationAngle, Qt::ZAxis).rotate(m_yRotationAngle, Qt::YAxis).rotate(m_xRotationAngle, Qt::XAxis));
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
    return m_dirtyTransforming;
}

bool AbstractContent::fromXml(QDomElement & pe)
{
    // restore content properties
    QDomElement domElement;

    // Load image size saved in the rect node (FIXME: move this later?)
    domElement = pe.firstChildElement("rect");
    int x, y, w, h;
    x = domElement.firstChildElement("x").text().toInt();
    y = domElement.firstChildElement("y").text().toInt();
    w = domElement.firstChildElement("w").text().toInt();
    h = domElement.firstChildElement("h").text().toInt();
    resizeContents(QRect(x, y, w, h));

    // Load position coordinates
    domElement = pe.firstChildElement("pos");
    x = domElement.firstChildElement("x").text().toInt();
    y = domElement.firstChildElement("y").text().toInt();
    setPos(x, y);

    int zvalue = pe.firstChildElement("zvalue").text().toInt();
    setZValue(zvalue);

    bool visible = pe.firstChildElement("visible").text().toInt();
    setVisible(visible);

    bool hasText = pe.firstChildElement("frame-text-enabled").text().toInt();
    setFrameTextEnabled(hasText);
    if (hasText) {
        QString text = pe.firstChildElement("frame-text").text();
        setFrameText(text);
    }

    quint32 frameClass = pe.firstChildElement("frame-class").text().toInt();
    setFrame(frameClass ? FrameFactory::createFrame(frameClass) : 0);

    // restore transformation
    QDomElement te = pe.firstChildElement("transformation");
    if (!te.isNull()) {
        QTransform t(te.attribute("m11").toDouble(), te.attribute("m12").toDouble(), te.attribute("m13").toDouble(),
                     te.attribute("m21").toDouble(), te.attribute("m22").toDouble(), te.attribute("m23").toDouble(),
                     te.attribute("m31").toDouble(), te.attribute("m32").toDouble(), te.attribute("m33").toDouble());
        setTransform(t);
    }
    return true;
}

void AbstractContent::toXml(QDomElement & pe) const
{
    // Save general item properties

    QDomDocument doc = pe.ownerDocument();
    QDomElement domElement;
    QDomText text;
    QString valueStr;

    // Save item position and size
    QDomElement rectParent = doc.createElement("rect");
    QDomElement xElement = doc.createElement("x");
    rectParent.appendChild(xElement);
    QDomElement yElement = doc.createElement("y");
    rectParent.appendChild(yElement);
    QDomElement wElement = doc.createElement("w");
    rectParent.appendChild(wElement);
    QDomElement hElement = doc.createElement("h");
    rectParent.appendChild(hElement);

    QRectF rect = m_contentsRect;
    xElement.appendChild(doc.createTextNode(QString::number(rect.left())));
    yElement.appendChild(doc.createTextNode(QString::number(rect.top())));
    wElement.appendChild(doc.createTextNode(QString::number(rect.width())));
    hElement.appendChild(doc.createTextNode(QString::number(rect.height())));
    pe.appendChild(rectParent);

    // Save the position
    domElement= doc.createElement("pos");
    xElement = doc.createElement("x");
    yElement = doc.createElement("y");
    valueStr.setNum(pos().x());
    xElement.appendChild(doc.createTextNode(valueStr));
    valueStr.setNum(pos().y());
    yElement.appendChild(doc.createTextNode(valueStr));
    domElement.appendChild(xElement);
    domElement.appendChild(yElement);
    pe.appendChild(domElement);

    // Save the stacking position
    domElement= doc.createElement("zvalue");
    pe.appendChild(domElement);
    valueStr.setNum(zValue());
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    // Save the visible state
    domElement= doc.createElement("visible");
    pe.appendChild(domElement);
    valueStr.setNum(isVisible());
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    // Save the frame class
    valueStr.setNum(frameClass());
    domElement= doc.createElement("frame-class");
    pe.appendChild(domElement);
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    domElement= doc.createElement("frame-text-enabled");
    pe.appendChild(domElement);
    valueStr.setNum(frameTextEnabled());
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    if(frameTextEnabled()) {
        domElement= doc.createElement("frame-text");
        pe.appendChild(domElement);
        text = doc.createTextNode(frameText());
        domElement.appendChild(text);
    }

    // save transformation
    const QTransform t = transform();
    if (!t.isIdentity()) {
        domElement = doc.createElement("transformation");
        domElement.setAttribute("m11", t.m11());
        domElement.setAttribute("m12", t.m12());
        domElement.setAttribute("m13", t.m13());
        domElement.setAttribute("m21", t.m21());
        domElement.setAttribute("m22", t.m22());
        domElement.setAttribute("m23", t.m23());
        domElement.setAttribute("m31", t.m31());
        domElement.setAttribute("m32", t.m32());
        domElement.setAttribute("m33", t.m33());
        pe.appendChild(domElement);
    }
}

QPixmap AbstractContent::renderAsBackground(const QSize & size, bool keepAspect) const
{
    QSize realSize = size;
    if (keepAspect) {
        int hfw = contentHeightForWidth(size.width());
        if (hfw > 1)
            realSize.setHeight(hfw);
    }
    QPixmap pix(realSize);
    pix.fill(Qt::transparent);
    return pix;
}

QRectF AbstractContent::boundingRect() const
{
    return m_frameRect;
}

void AbstractContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (!m_frame)
        return;

    // draw the Frame
    bool opaqueContent = contentOpaque();
    bool drawSelection = RenderOpts::HQRendering ? false : isSelected();
    QRect frameRect = m_frameRect.toRect();
    m_frame->paint(painter, frameRect, drawSelection, opaqueContent);

    // use clip path for contents, if set
    if (m_frame->clipContents())
        painter->setClipPath(m_frame->contentsClipPath(m_contentsRect));
}

void AbstractContent::GFX_CHANGED() const
{
    if (m_gfxChangeTimer)
        m_gfxChangeTimer->start();
}

int AbstractContent::contentHeightForWidth(int width) const
{
    return width;
}

bool AbstractContent::contentOpaque() const
{
    return false;
}

void AbstractContent::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (CornerItem * corner, m_cornerItems)
        corner->show();
    foreach (ButtonItem * button, m_controlItems)
        button->show();
}

void AbstractContent::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    foreach (CornerItem * corner, m_cornerItems)
        corner->hide();
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
    QGraphicsItem::mousePressEvent(event);
}

void AbstractContent::keyPressEvent(QKeyEvent * event)
{
    event->accept();
    if (event->key() == Qt::Key_Delete)
        emit deleteItem();
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

void AbstractContent::createCorner(Qt::Corner corner, bool noRescale)
{
    CornerItem * c = new CornerItem(corner, noRescale, this);
    //c->setToolTip(tr("Hold down SHIFT for ignoring aspect ratio.\nDouble click to restore the aspect ratio."));
    m_cornerItems.append(c);
}

void AbstractContent::layoutChildren()
{
    // layout corners
    foreach (CornerItem * corner, m_cornerItems)
        corner->relayout(m_contentsRect);

    // layout buttons even if no frame
    if (!m_frame) {
        int right = m_frameRect.right() - 12;
        int bottom = m_frameRect.bottom() + 2; // if no frame, offset the buttons a little on bottom
        foreach (ButtonItem * button, m_controlItems) {
            button->setPos(right - button->width() / 2, bottom - button->height() / 2);
            right -= button->width() + 4;
        }
        return;
    }

    // layout all controls
    m_frame->layoutButtons(m_controlItems, m_frameRect.toRect());

    // layout text, if present
    if (m_frameTextItem)
        m_frame->layoutText(m_frameTextItem, m_frameRect.toRect());
}

void AbstractContent::slotPerspective(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers)
{
    ButtonItem * button = static_cast<ButtonItem *>(sender());
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = button->pos();
    if (newPos == refPos)
        return;

    int march=0;
    if(modifiers == Qt::NoModifier) march = 2;
    else if( modifiers == Qt::ControlModifier ) {
        m_xRotationAngle=0;
        m_yRotationAngle=0;
    }
    else march = 4;

    //// Perspective : move along X axis
    if(newPos.y() - refPos.y() > 70) m_xRotationAngle -= march;
    else if(newPos.y() - refPos.y() < -70) m_xRotationAngle += march;
    // Prevents the user from rotating too much (if so, buttons become unavailable).
    if (m_xRotationAngle > 80) m_xRotationAngle = 80;
    if (m_xRotationAngle < -80 ) m_xRotationAngle = -80;

    /// Move along Y axis
    if(newPos.x() - refPos.x() > 70) m_yRotationAngle -= march;
    else if(newPos.x() - refPos.x() < -70) m_yRotationAngle += march;
    if (m_yRotationAngle > 80) m_yRotationAngle = 80;
    if (m_yRotationAngle < -80 ) m_yRotationAngle = -80;

    setTransform(QTransform().rotate(m_zRotationAngle, Qt::ZAxis).rotate(m_yRotationAngle, Qt::YAxis).rotate(m_xRotationAngle, Qt::XAxis));
}

void AbstractContent::slotDirtyEnded()
{
    m_dirtyTransforming = false;
    update();
    GFX_CHANGED();
}
