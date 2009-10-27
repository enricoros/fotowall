/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
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

#include "Frames/FrameFactory.h"
#include "Shared/RenderOpts.h"
#include "ButtonItem.h"
#include "CornerItem.h"
#include "MirrorItem.h"

#include <QApplication>
#include <QDate>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <math.h>

AbstractContent::AbstractContent(QGraphicsScene * scene, QGraphicsItem * parent, bool noRescale)
    : AbstractDisposeable(parent, false /*needed for Hardware3DTest*/)
    , m_contentRect(-100, -75, 200, 150)
    , m_frame(0)
    , m_frameTextItem(0)
    , m_controlsVisible(false)
    , m_dirtyTransforming(false)
    , m_transformRefreshTimer(0)
    , m_gfxChangeTimer(0)
    , m_mirrorItem(0)
#if QT_VERSION < 0x040600
    , m_rotationAngle(0)
#endif
{
    // the buffered graphics changes timer
    m_gfxChangeTimer = new QTimer(this);
    m_gfxChangeTimer->setInterval(0);
    m_gfxChangeTimer->setSingleShot(true);

    // customize item's behavior
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
    // allow some items (eg. the shape controls for text) to be shown
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptHoverEvents(true);

    // create child controls
    createCorner(Qt::TopLeftCorner, noRescale);
    createCorner(Qt::TopRightCorner, noRescale);
    createCorner(Qt::BottomLeftCorner, noRescale);
    createCorner(Qt::BottomRightCorner, noRescale);

    //ButtonItem * bFront = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-order-front.png"), this);
    //bFront->setToolTip(tr("Raise"));
    //connect(bFront, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
    //addButtonItem(bFront);

    ButtonItem * bConf = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-configure.png"), this);
    bConf->setToolTip(tr("Change properties..."));
    connect(bConf, SIGNAL(clicked()), this, SLOT(slotConfigure()));
    addButtonItem(bConf);

#if QT_VERSION >= 0x040500
    ButtonItem * bPersp = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-perspective.png"), this);
    bPersp->setToolTip(tr("Drag around to change the perspective.\nHold SHIFT to move faster.\nUse CTRL to cancel the transformations."));
    connect(bPersp, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotSetPerspective(const QPointF&,Qt::KeyboardModifiers)));
    connect(bPersp, SIGNAL(doubleClicked()), this, SLOT(slotClearPerspective()));
    addButtonItem(bPersp);
#endif

    ButtonItem * bDelete = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
    bDelete->setSelectsParent(false);
    bDelete->setToolTip(tr("Remove"));
    connect(bDelete, SIGNAL(clicked()), this, SIGNAL(requestRemoval()));
    addButtonItem(bDelete);

    // create default frame
    Frame * frame = FrameFactory::defaultPictureFrame();
    setFrame(frame);

    // hide and layoutChildren buttons
    layoutChildren();

    // add to the scene
    scene->addItem(this);

    // display mirror
#if QT_VERSION >= 0x040600
    // WORKAROUND with Qt 4.6-tp1 there are crashes activating a mirror before setting the scene
    // need to rethink this anyway
    setMirrored(false);
#else
    setMirrored(RenderOpts::LastMirrored);
#endif
}

AbstractContent::~AbstractContent()
{
    qDeleteAll(m_cornerItems);
    qDeleteAll(m_controlItems);
    delete m_mirrorItem;
    delete m_frameTextItem;
    delete m_frame;
}

void AbstractContent::dispose()
{
    // stick this item
    setFlags((GraphicsItemFlags)0x00);

    // fade out mirror too
    setMirrored(false);

    // little rotate animation
#if QT_VERSION >= 0x040600
    QPropertyAnimation * ani = new QPropertyAnimation(this, "rotation");
    ani->setEasingCurve(QEasingCurve::InQuad);
    ani->setDuration(300);
    ani->setEndValue(-30.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#endif

    // standard disposition
    AbstractDisposeable::dispose();
}

QRect AbstractContent::contentRect() const
{
    return m_contentRect;
}

void AbstractContent::resizeContents(const QRect & rect, bool keepRatio)
{
    if (!rect.isValid())
        return;

    prepareGeometryChange();

    m_contentRect = rect;
    if (keepRatio) {
        int hfw = contentHeightForWidth(rect.width());
        if (hfw > 1) {
            m_contentRect.setTop(-hfw / 2);
            m_contentRect.setHeight(hfw);
        }
    }

    m_frameRect = m_frame ? m_frame->frameRect(m_contentRect) : m_contentRect;

    layoutChildren();
    update();
    GFX_CHANGED();
}

void AbstractContent::resetContentsRatio()
{
    resizeContents(m_contentRect, true);
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
    FrameFactory::setDefaultPictureClass(frameClass());
    if (!m_frame && m_frameTextItem)
        m_frameTextItem->hide();
    resizeContents(m_contentRect);
    layoutChildren();
    update();
    GFX_CHANGED();
}

quint32 AbstractContent::frameClass() const
{
    return m_frame ? m_frame->frameClass() : (quint32)Frame::NoFrame;
}

#include <QGraphicsTextItem>
class MyTextItem : public QGraphicsTextItem {
    public:
        MyTextItem(QGraphicsItem * parent = 0)
            : QGraphicsTextItem(parent)
        {
        }

        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 )
        {
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing, true);
            QGraphicsTextItem::paint(painter, option, widget);
            painter->restore();
        }

        // prevent the TextItem from listening to global shortcuts
        bool eventFilter(QObject * object, QEvent * event)
        {
            if (event->type() == QEvent::Shortcut || event->type() == QEvent::ShortcutOverride) {
                if (!object->inherits("QGraphicsView")) {
                    event->accept();
                    return true;
                }
            }
            return false;
        }

    protected:
        void focusInEvent(QFocusEvent * event)
        {
            QGraphicsTextItem::focusInEvent(event);
            qApp->installEventFilter(this);
        }

        void focusOutEvent(QFocusEvent * event)
        {
            QGraphicsTextItem::focusOutEvent(event);
            qApp->removeEventFilter(this);
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
        m_frameTextItem->setZValue(1.0);
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

void AbstractContent::setFrameTextReadonly(bool read)
{
    if (m_frameTextItem)
        m_frameTextItem->setTextInteractionFlags(read ? Qt::NoTextInteraction : Qt::TextEditorInteraction);
}

bool AbstractContent::frameTextReadonly() const
{
    if (!m_frameTextItem)
        return false;
    return m_frameTextItem->textInteractionFlags() == Qt::NoTextInteraction;
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
    button->setVisible(m_controlsVisible);
    button->setZValue(3.0);
    layoutChildren();
}

void AbstractContent::setMirrored(bool enabled)
{
    if (m_mirrorItem && !enabled) {
        m_mirrorItem->dispose();
        m_mirrorItem = 0;
        emit mirroredChanged();
    }
    if (enabled && !m_mirrorItem) {
        m_mirrorItem = new MirrorItem(this);
        connect(m_gfxChangeTimer, SIGNAL(timeout()), m_mirrorItem, SLOT(sourceChanged()));
        connect(this, SIGNAL(destroyed()), m_mirrorItem, SLOT(deleteLater()));
        emit mirroredChanged();
    }
}

bool AbstractContent::mirrored() const
{
    return m_mirrorItem;
}

void AbstractContent::setPerspective(const QPointF & angles)
{
    if (angles != m_perspectiveAngles) {
        m_perspectiveAngles = angles;
        applyTransforms();
        emit perspectiveChanged();
    }
}

QPointF AbstractContent::perspective() const
{
    return m_perspectiveAngles;
}

#if QT_VERSION < 0x040600
void AbstractContent::setRotation(qreal angle)
{
    if (m_rotationAngle != angle) {
        m_rotationAngle = angle;
        applyTransforms();
        emit rotationChanged();
    }
}

qreal AbstractContent::rotation() const
{
    return m_rotationAngle;
}
#endif

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

QWidget * AbstractContent::createPropertyWidget()
{
    return 0;
}

bool AbstractContent::fromXml(QDomElement & contentElement)
{
    // restore content properties
    QDomElement domElement;

    // Load image size saved in the rect node
    domElement = contentElement.firstChildElement("rect");
    qreal x, y, w, h;
    x = domElement.firstChildElement("x").text().toDouble();
    y = domElement.firstChildElement("y").text().toDouble();
    w = domElement.firstChildElement("w").text().toDouble();
    h = domElement.firstChildElement("h").text().toDouble();
    resizeContents(QRect(x, y, w, h));

    // Load position coordinates
    domElement = contentElement.firstChildElement("pos");
    x = domElement.firstChildElement("x").text().toDouble();
    y = domElement.firstChildElement("y").text().toDouble();
    setPos(x, y);

    int zvalue = contentElement.firstChildElement("zvalue").text().toDouble();
    setZValue(zvalue);

    bool visible = contentElement.firstChildElement("visible").text().toInt();
    setVisible(visible);

    bool hasText = contentElement.firstChildElement("frame-text-enabled").text().toInt();
    setFrameTextEnabled(hasText);
    if (hasText) {
        QString text = contentElement.firstChildElement("frame-text").text();
        setFrameText(text);
    }

    quint32 frameClass = contentElement.firstChildElement("frame-class").text().toInt();
    setFrame(frameClass ? FrameFactory::createFrame(frameClass) : 0);

    // restore transformation
    QDomElement te = contentElement.firstChildElement("transformation");
    if (!te.isNull()) {
        m_perspectiveAngles = QPointF(te.attribute("xRot").toDouble(), te.attribute("yRot").toDouble());
#if QT_VERSION < 0x040600
        m_rotationAngle = te.attribute("zRot").toDouble();
#else
        setRotation(te.attribute("zRot").toDouble());
#endif
        applyTransforms();
    }
    domElement = contentElement.firstChildElement("mirror");
    setMirrored(domElement.attribute("state").toInt());

    return true;
}

void AbstractContent::toXml(QDomElement & contentElement) const
{
    // Save general item properties
    contentElement.setTagName("abstract");
    QDomDocument doc = contentElement.ownerDocument();
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

    QRectF rect = m_contentRect;
    xElement.appendChild(doc.createTextNode(QString::number(rect.left())));
    yElement.appendChild(doc.createTextNode(QString::number(rect.top())));
    wElement.appendChild(doc.createTextNode(QString::number(rect.width())));
    hElement.appendChild(doc.createTextNode(QString::number(rect.height())));
    contentElement.appendChild(rectParent);

    // Save the position
    domElement = doc.createElement("pos");
    xElement = doc.createElement("x");
    yElement = doc.createElement("y");
    valueStr.setNum(pos().x());
    xElement.appendChild(doc.createTextNode(valueStr));
    valueStr.setNum(pos().y());
    yElement.appendChild(doc.createTextNode(valueStr));
    domElement.appendChild(xElement);
    domElement.appendChild(yElement);
    contentElement.appendChild(domElement);

    // Save the stacking position
    domElement= doc.createElement("zvalue");
    contentElement.appendChild(domElement);
    valueStr.setNum(zValue());
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    // Save the visible state
    domElement= doc.createElement("visible");
    contentElement.appendChild(domElement);
    valueStr.setNum(isVisible());
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    // Save the frame class
    valueStr.setNum(frameClass());
    domElement= doc.createElement("frame-class");
    contentElement.appendChild(domElement);
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    domElement= doc.createElement("frame-text-enabled");
    contentElement.appendChild(domElement);
    valueStr.setNum(frameTextEnabled());
    text = doc.createTextNode(valueStr);
    domElement.appendChild(text);

    if(frameTextEnabled()) {
        domElement= doc.createElement("frame-text");
        contentElement.appendChild(domElement);
        text = doc.createTextNode(frameText());
        domElement.appendChild(text);
    }

    // save transformation
    const QTransform t = transform();
    if (!t.isIdentity() || rotation() != 0) {
        domElement = doc.createElement("transformation");
        domElement.setAttribute("xRot", m_perspectiveAngles.x());
        domElement.setAttribute("yRot", m_perspectiveAngles.y());
#if QT_VERSION < 0x040600
        domElement.setAttribute("zRot", m_rotationAngle);
#else
        domElement.setAttribute("zRot", rotation());
#endif
        contentElement.appendChild(domElement);
    }
    domElement = doc.createElement("mirror");
    domElement.setAttribute("state", mirrored());
    contentElement.appendChild(domElement);

}

QPixmap AbstractContent::toPixmap(const QSize & size, Qt::AspectRatioMode ratio)
{
    Q_UNUSED(ratio)
    // allocate a pixmap and draw the content over it (NOTE: aspect ratio is ignored)
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    drawContent(&painter, pixmap.rect());
    painter.end();
    return pixmap;
}

QRectF AbstractContent::boundingRect() const
{
    return m_frameRect;
}

void AbstractContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    // emitting the edit request by default. some subclasses request backgrounding
    emit requestEditing();
}

void AbstractContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // find out whether to draw the selection
    const bool drawSelection = RenderOpts::HQRendering ? false : isSelected();

    if (m_frame) {
        // draw the Frame
        m_frame->drawFrame(painter, m_frameRect.toRect(), drawSelection, contentOpaque());

        // use clip path for contents, if set
        if (m_frame->clipContents())
            painter->setClipPath(m_frame->contentsClipPath(m_contentRect));
    }

#if 0
    if (RenderOpts::OpenGLWindow && drawSelection)
        painter->setCompositionMode(QPainter::CompositionMode_Plus);
#endif

    // paint the inner contents
    const QRect tcRect = QRect(0, 0, m_contentRect.width(), m_contentRect.height());
    painter->translate(m_contentRect.topLeft());
    drawContent(painter, tcRect);

    // overlay a selection
    if (drawSelection && !m_frame) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(RenderOpts::hiColor, 2.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(tcRect);
    }
}

void AbstractContent::selectionChanged(bool /*selected*/)
{
    // nothing to do here.. only used by subclasses
}

void AbstractContent::GFX_CHANGED() const
{
    if (m_gfxChangeTimer && m_mirrorItem)
        m_gfxChangeTimer->start();
}

void AbstractContent::setControlsVisible(bool visible)
{
    m_controlsVisible = visible;
    foreach (CornerItem * corner, m_cornerItems)
        corner->setVisible(visible);
    foreach (ButtonItem * button, m_controlItems)
        button->setVisible(visible);
}

QPixmap AbstractContent::ratioScaledPixmap(const QPixmap * source, const QSize & size, Qt::AspectRatioMode ratio) const
{
    QPixmap scaledPixmap = source->scaled(size, ratio, Qt::SmoothTransformation);
    if (scaledPixmap.size() != size) {
        int offX = (scaledPixmap.width() - size.width()) / 2;
        int offY = (scaledPixmap.height() - size.height()) / 2;
        if (ratio == Qt::KeepAspectRatio) {
            QPixmap rightSizePixmap(size);
            rightSizePixmap.fill(Qt::transparent);
            QPainter p(&rightSizePixmap);
            p.drawPixmap(-offX, -offY, scaledPixmap);
            p.end();
            return rightSizePixmap;
        }
        if (ratio == Qt::KeepAspectRatioByExpanding) {
            return scaledPixmap.copy(offX, offY, size.width(), size.height());
        }
    }
    return scaledPixmap;
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
    setControlsVisible(true);
}

void AbstractContent::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    setControlsVisible(false);
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
    QGraphicsItem::mousePressEvent(event);
    if (event->button() == Qt::RightButton) {
        setSelected(true);
        emit requestConfig(event->scenePos().toPoint());
    }
}

void AbstractContent::keyPressEvent(QKeyEvent * event)
{
    // discard key events for unselectable items
    if (!(flags() & ItemIsSelectable)) {
        event->ignore();
        return;
    }
    event->accept();
    int step = (event->modifiers() & Qt::ShiftModifier) ? 50 : (event->modifiers() & Qt::ControlModifier ? 1 : 10);
    switch (event->key()) {
        // cursor keys: 10px, 50 if Shift pressed, 1 if Control pressed
        case Qt::Key_Left:      setPos(pos() - QPointF(step, 0));   break;
        case Qt::Key_Up:        setPos(pos() - QPointF(0, step));   break;
        case Qt::Key_Right:     setPos(pos() + QPointF(step, 0));   break;
        case Qt::Key_Down:      setPos(pos() + QPointF(0, step));   break;

        // deletion
        case Qt::Key_Delete: emit requestRemoval(); break;

        // editing
        case Qt::Key_Return: emit requestEditing(); break;

        // ignore unhandled key
        default: event->ignore();
    }
}

QVariant AbstractContent::itemChange(GraphicsItemChange change, const QVariant & value)
{
    // keep the AbstractContent's center inside the scene rect..
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        QRectF sceneRect = scene()->sceneRect();
        if (!sceneRect.contains(newPos) /*&& rect.width() > 100 && rect.height() > 100*/) {
            newPos.setX(qBound(sceneRect.left(), newPos.x(), sceneRect.right()));
            newPos.setY(qBound(sceneRect.top(), newPos.y(), sceneRect.bottom()));
            return newPos;
        }
    }

    // tell subclasses about selection changes
    if (change == ItemSelectedHasChanged)
        selectionChanged(value.toBool());

    // changes that affect the mirror item
    if (m_mirrorItem) {
        switch (change) {
            // notify about setPos
            case ItemPositionHasChanged:
                m_mirrorItem->sourceMoved();
                break;

            // notify about graphics changes
            //case ItemMatrixChange:
            //case ItemTransformChange:
            case ItemTransformHasChanged:
            case ItemEnabledHasChanged:
            case ItemSelectedHasChanged:
            case ItemParentHasChanged:
#if QT_VERSION >= 0x040500
            case ItemOpacityHasChanged:
#endif
                GFX_CHANGED();
                break;

            case ItemZValueHasChanged:
                m_mirrorItem->setZValue(zValue());
                break;

            case ItemVisibleHasChanged:
                m_mirrorItem->setVisible(isVisible());
                break;

            default:
                break;
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
    emit requestConfig(item->scenePos().toPoint());
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
    // make up the default save path (stored as 'Fotowall/ExportDir')
    QSettings s;
    QString defaultSavePath = tr("Unnamed %1.png").arg(QDate::currentDate().toString());
    if (s.contains("Fotowall/ExportDir"))
        defaultSavePath.prepend(s.value("Fotowall/ExportDir").toString() + QDir::separator());

    // ask the file name, validate it, store back to settings
    QString fileName = QFileDialog::getSaveFileName(0, tr("Choose the Image file"), defaultSavePath, tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if (fileName.isNull())
        return;
    s.setValue("Fotowall/ExportDir", QFileInfo(fileName).absolutePath());
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
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
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
    c->setVisible(m_controlsVisible);
    c->setZValue(2.0);
    c->setToolTip(tr("Drag with Left or Right mouse button.\n - Hold down SHIFT for free resize\n - Hold down CTRL to allow rotation\n - Hold down ALT to snap rotation\n - Double click (with LMB/RMB) to restore the aspect ratio/rotation"));
    m_cornerItems.append(c);
}

void AbstractContent::layoutChildren()
{
    // layout corners
    foreach (CornerItem * corner, m_cornerItems)
        corner->relayout(m_contentRect);

    // layout controls and text
    if (m_frame) {
        m_frame->layoutButtons(m_controlItems, m_frameRect.toRect());
        if (m_frameTextItem)
            m_frame->layoutText(m_frameTextItem, m_frameRect.toRect());
    } else {
        // layout controls
        const int spacing = 4;
        int left = m_frameRect.left();
        int top = m_frameRect.top();
        int right = m_frameRect.right();
        int bottom = m_frameRect.bottom();
        int offset = right;
        foreach (ButtonItem * button, m_controlItems) {
            switch (button->buttonType()) {
                case ButtonItem::FlipH:
                    button->setPos(right - button->width() / 2, (top + bottom) / 2);
                    break;

                case ButtonItem::FlipV:
                    button->setPos((left + right) / 2, top + button->height() / 2);
                    break;

                default:
                    button->setPos(offset - button->width() / 2, bottom - button->height() / 2);
                    offset -= button->width() + spacing;
                    break;
            }
        }
        // hide text, if present
        if (m_frameTextItem)
            m_frameTextItem->hide();
    }
}

void AbstractContent::applyTransforms()
{
    setTransform(QTransform().rotate(m_perspectiveAngles.y(), Qt::XAxis)
                 .rotate(m_perspectiveAngles.x(), Qt::YAxis)
#if QT_VERSION < 0x040600
                 .rotate(m_rotationAngle, Qt::ZAxis)
#endif
                 , false);
}

void AbstractContent::slotSetPerspective(const QPointF & sceneRelPoint, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier)
        return slotClearPerspective();
    qreal k = modifiers == Qt::NoModifier ? 0.2 : 0.5;
    setPerspective(QPointF(qBound((qreal)-70.0, (qreal)sceneRelPoint.x()*k, (qreal)70.0), qBound((qreal)-70.0, (qreal)sceneRelPoint.y()*k, (qreal)70.0)));
}

void AbstractContent::slotClearPerspective()
{
    setPerspective(QPointF(0, 0));
}

void AbstractContent::slotDirtyEnded()
{
    m_dirtyTransforming = false;
    update();
    GFX_CHANGED();
}
