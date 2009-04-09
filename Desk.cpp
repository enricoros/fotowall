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

#include "Desk.h"
#include "ColorPickerItem.h"
#include "HelpItem.h"
#include "HighlightItem.h"
#include "PictureContent.h"
#include "TextContent.h"
#include "PictureProperties.h"
#include "TextProperties.h"
#include "frames/FrameFactory.h"
#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <QImageReader>
#include <QInputDialog>
#include <QMenu>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QFile>

#define COLORPICKER_W 200
#define COLORPICKER_H 150

Desk::Desk(QObject * parent)
    : QGraphicsScene(parent)
    , m_helpItem(0)
    , m_backContent(0)
    , m_topBarEnabled(true)
    , m_bottomBarEnabled(false)
{
    // create colorpickers
    m_titleColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_titleColorPicker->setColor(Qt::gray);
    m_titleColorPicker->setAnimated(true);
    m_titleColorPicker->setAnchor(ColorPickerItem::AnchorTop);
    m_titleColorPicker->setZValue(10000);
    m_titleColorPicker->setVisible(false);
    connect(m_titleColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotTitleColorChanged()));
    addItem(m_titleColorPicker);

    m_foreColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_foreColorPicker->setColor(QColor(128, 128, 128));
    m_foreColorPicker->setAnimated(true);
    m_foreColorPicker->setAnchor(ColorPickerItem::AnchorTopLeft);
    m_foreColorPicker->setZValue(10000);
    connect(m_foreColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotForeColorChanged()));
    addItem(m_foreColorPicker);

    m_grad1ColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_grad1ColorPicker->setColor(QColor(192, 192, 192));
    m_grad1ColorPicker->setAnimated(true);
    m_grad1ColorPicker->setAnchor(ColorPickerItem::AnchorTopRight);
    m_grad1ColorPicker->setZValue(10000);
    connect(m_grad1ColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotGradColorChanged()));
    addItem(m_grad1ColorPicker);

    m_grad2ColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_grad2ColorPicker->setColor(QColor(80, 80, 80));
    m_grad2ColorPicker->setAnimated(true);
    m_grad2ColorPicker->setAnchor(ColorPickerItem::AnchorBottomRight);
    m_grad2ColorPicker->setZValue(10000);
    connect(m_grad2ColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotGradColorChanged()));
    addItem(m_grad2ColorPicker);
}

Desk::~Desk()
{
    qDeleteAll(m_highlightItems);
    delete m_helpItem;
    delete m_titleColorPicker;
    delete m_foreColorPicker;
    delete m_grad1ColorPicker;
    delete m_grad2ColorPicker;
    qDeleteAll(m_content);
    m_content.clear();
    m_backContent = 0;
}

/// Add Content
void Desk::addPictures(const QStringList & fileNames)
{
    QPoint pos = sceneRect().center().toPoint();
    foreach (const QString & localFile, fileNames) {
        if (!QFile::exists(localFile))
            continue;

        // create picture and load the file
        PictureContent * p = createPicture(pos);
        if (!p->loadPhoto(localFile, true, true)) {
            m_content.removeAll(p);
            delete p;
        } else
            pos += QPoint(30, 30);
    }
}

void Desk::addText()
{
    createText(sceneRect().center().toPoint());
}

/// resize Desk
void Desk::resize(const QSize & size)
{
    // relayout contents
    m_size = size;
    m_rect = QRectF(0, 0, m_size.width(), m_size.height());
    m_titleColorPicker->setPos((size.width() - COLORPICKER_W) / 2.0, 10);
    m_grad1ColorPicker->setPos(size.width() - COLORPICKER_W, 0);
    m_grad2ColorPicker->setPos(size.width() - COLORPICKER_W, size.height() - COLORPICKER_H);
    if (m_helpItem)
        m_helpItem->setPos(m_rect.center().toPoint());
    foreach (HighlightItem * highlight, m_highlightItems)
        highlight->reposition(m_rect);

    // ensure visibility
    foreach (AbstractContent * content, m_content)
        content->ensureVisible(m_rect);
    foreach (AbstractProperties * properties, m_properties)
        properties->keepInBoundaries(m_rect.toRect());

    // change my rect
    setSceneRect(m_rect);
}

/// Title
QString Desk::titleText() const
{
    return m_titleText;
}

void Desk::setTitleText(const QString & text)
{
    m_titleText = text;
    m_titleColorPicker->setVisible(!text.isEmpty());
    update(0, 0, m_size.width(), 50);
}

/// Misc: save, restore, help...
#define HIGHLIGHT(x, y) \
    { \
        HighlightItem * highlight = new HighlightItem(); \
        m_highlightItems.append(highlight); \
        addItem(highlight); \
        highlight->setZValue(10000); \
        highlight->setPosF(x, y); \
        highlight->show(); \
    }

void Desk::showHelp()
{
    if (m_helpItem)
        return;

    // help item
    m_helpItem = new HelpItem();
    connect(m_helpItem, SIGNAL(closeMe()), this, SLOT(slotCloseHelp()));
    addItem(m_helpItem);
    m_helpItem->setZValue(10001);
    m_helpItem->setPos(sceneRect().center().toPoint());
    m_helpItem->show();

    // blink items
    HIGHLIGHT(0.0, 0.0);
    HIGHLIGHT(0.5, 0.0);
    HIGHLIGHT(1.0, 0.0);
    HIGHLIGHT(1.0, 1.0);
}

void Desk::save(QDataStream & data) const
{
    // FIXME: move to a serious XML format ...

    // save own data
    data << m_titleColorPicker->color();
    data << m_foreColorPicker->color();
    data << m_grad1ColorPicker->color();
    data << m_grad2ColorPicker->color();
    data << m_titleText;

    // TODO: save background

    // save the contents
    ///data << m_content.size();
    foreach (AbstractContent * content, m_content) {
        // write the content type
        int type = 0;
        if (content->inherits("PictureContent"))
            type = 1;
        else if (content->inherits("TextContent"))
            type = 2;
        else {
            qWarning("Desk::save: error saving data");
            continue;
        }
        data << type;

        // write the content payload
        content->save(data);
    }
}

void Desk::restore(QDataStream & data)
{
    // FIXME: move to a serious XML format ...

    // restore own data
    QColor color;
    data >> color;
    m_titleColorPicker->setColor(color);
    data >> color;
    m_foreColorPicker->setColor(color);
    data >> color;
    m_grad1ColorPicker->setColor(color);
    data >> color;
    m_grad2ColorPicker->setColor(color);
    QString titleText;
    data >> titleText;
    setTitleText(titleText);

    // FIXME: restore background

    // restore the content
    qDeleteAll(m_content);
    m_content.clear();
    m_backContent = 0;
    while (!data.atEnd()) {
        int type;
        data >> type;
        AbstractContent * content = 0;
        switch (type) {
            case 1:
                content = createPicture(QPoint());
                break;
            case 2:
                content = createText(QPoint());
                break;
            default:
                qWarning("Desk::restore: error loading data");
                continue;
        }
        if (!content->restore(data)) {
            m_content.removeAll(content);
            delete content;
        }
    }
    update();
}

/// Drag & Drop image files
void Desk::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children but accept it only for image files
    QGraphicsScene::dragEnterEvent(event);
    event->ignore();

    // skip bad mimes
    if (!event->mimeData() || !event->mimeData()->hasUrls())
        return;

    // get supported images extensions
    QStringList extensions;
    foreach (const QByteArray & format, QImageReader::supportedImageFormats())
        extensions.append( "." + format );

    // match each image file with urls
    foreach (const QUrl & url, event->mimeData()->urls()) {
        QString localFile = url.toLocalFile();
        foreach (const QString & extension, extensions) {
            if (localFile.endsWith(extension, Qt::CaseInsensitive)) {
                event->accept();
                return;
            }
        }
    }
}

void Desk::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children
    event->ignore();
    QGraphicsScene::dragMoveEvent(event);

    // or accept event for the Desk
    if (!event->isAccepted()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void Desk::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children
    event->ignore();
    QGraphicsScene::dropEvent(event);
    if (event->isAccepted())
        return;

    // or handle as a Desk drop event
    event->accept();
    QPoint pos = event->scenePos().toPoint();
    foreach (const QUrl & url, event->mimeData()->urls()) {
        QString localFile = url.toLocalFile();
        if (!QFile::exists(localFile))
            continue;

        // create PictureContent from file
        PictureContent * p = createPicture(pos);
        if (!p->loadPhoto(localFile, true, true)) {
            m_content.removeAll(p);
            delete p;
        } else
            pos += QPoint(30, 30);
    }
}

void Desk::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    // first dispatch doubleclick to items
    mouseEvent->ignore();
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    // unset the background picture, if present
    if (m_backContent) {
        m_backContent->show();
        m_backContent = 0;
        m_backCache = QPixmap();
        update();
    }
}

void Desk::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
    // context menu on empty area
    if (items(event->scenePos()).isEmpty()) {
        QMenu popup;
        popup.setSeparatorsCollapsible(false);
        //popup.addSeparator()->setText( ... );

        QAction * aSetTitle = new QAction(tr("Set title..."), &popup);
        connect(aSetTitle, SIGNAL(triggered()), this, SLOT(slotSetTitle()));
        popup.addAction(aSetTitle);

        QAction * aClearTitle = new QAction(tr("Clear title"), &popup);
        connect(aClearTitle, SIGNAL(triggered()), this, SLOT(slotClearTitle()));
        popup.addAction(aClearTitle);

        QAction * aTop = new QAction(tr("Top bar"), &popup);
        aTop->setCheckable(true);
        aTop->setChecked(m_topBarEnabled);
        connect(aTop, SIGNAL(toggled(bool)), this, SLOT(slotSetTopBarEnabled(bool)));
        popup.addAction(aTop);

        QAction * aBottom = new QAction(tr("Bottom bar"), &popup);
        aBottom->setCheckable(true);
        aBottom->setChecked(m_bottomBarEnabled);
        connect(aBottom, SIGNAL(toggled(bool)), this, SLOT(slotSetBottomBarEnabled(bool)));
        popup.addAction(aBottom);

        popup.exec(event->screenPos());
    }
}


/// Scene Background & Foreground
void Desk::drawBackground(QPainter * painter, const QRectF & rect)
{
    // draw content if set
    if (m_backContent) {
        // regenerate cache if needed
        QSize sceneSize = sceneRect().size().toSize();
        if (m_backCache.isNull() || m_backCache.size() != sceneSize)
            m_backCache = m_backContent->renderAsBackground(sceneSize);

        // paint cached background
        QRect targetRect = rect.toRect();
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawPixmap(targetRect, m_backCache, targetRect);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        return;
    }

    // draw background
    QLinearGradient lg(m_rect.topLeft(), m_rect.bottomLeft());
    lg.setColorAt(0.0, m_grad1ColorPicker->color());
    lg.setColorAt(1.0, m_grad2ColorPicker->color());
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void Desk::drawForeground(QPainter * painter, const QRectF & /*rect*/)
{
    // draw header/footer
    if (m_topBarEnabled || m_bottomBarEnabled) {
        QColor hColor = m_foreColorPicker->color();
        hColor.setAlpha(128);
        if (m_topBarEnabled)
            painter->fillRect(0, 0, m_size.width(), 50, hColor);
        if (m_bottomBarEnabled)
            painter->fillRect(0, m_size.height() - 50, m_size.width(), 50, hColor);
    }

    // draw text
    painter->setFont(QFont("Courier 10 Pitch", 28));
    QLinearGradient lg(0,15,0,35);
    QColor titleColor = m_titleColorPicker->color();
    lg.setColorAt(0.0, titleColor);
    lg.setColorAt(0.49, titleColor.lighter(150));
    lg.setColorAt(0.51, titleColor.darker(150));
    lg.setColorAt(1.0, titleColor);
    painter->setPen(QPen(lg, 0));
    painter->drawText(QRect(0, 0, m_size.width(), 50), Qt::AlignCenter, m_titleText);
}

PictureContent * Desk::createPicture(const QPoint & pos)
{
    PictureContent * p = new PictureContent(this);
    connect(p, SIGNAL(configureMe(const QPoint &)), this, SLOT(slotConfigureContent(const QPoint &)));
    connect(p, SIGNAL(backgroundMe()), this, SLOT(slotBackgroundContent()));
    connect(p, SIGNAL(changeStack(int)), this, SLOT(slotStackContent(int)));
    connect(p, SIGNAL(deleteMe()), this, SLOT(slotDeleteContent()));
    //p->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    p->setPos(pos);
    p->setZValue(m_content.isEmpty() ? 1 : (m_content.last()->zValue() + 1));
    p->show();
    m_content.append(p);
    return p;
}

TextContent * Desk::createText(const QPoint & pos)
{
    TextContent * t = new TextContent(this);
    connect(t, SIGNAL(configureMe(const QPoint &)), this, SLOT(slotConfigureContent(const QPoint &)));
    connect(t, SIGNAL(backgroundMe()), this, SLOT(slotBackgroundContent()));
    connect(t, SIGNAL(changeStack(int)), this, SLOT(slotStackContent(int)));
    connect(t, SIGNAL(deleteMe()), this, SLOT(slotDeleteContent()));
    //t->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    t->setPos(pos);
    t->setZValue(m_content.isEmpty() ? 1 : (m_content.last()->zValue() + 1));
    t->show();
    m_content.append(t);
    return t;
}

/// Slots
void Desk::slotConfigureContent(const QPoint & scenePoint)
{
    // get the content and ensure it has no
    AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
    foreach (AbstractProperties * properties, m_properties) {
        if (properties->content() == content) {
            properties->keepInBoundaries(sceneRect().toRect());
            return;
        }
    }
    AbstractProperties * p = 0;

    // picture properties (dialog and connections)
    PictureContent * picture = dynamic_cast<PictureContent *>(content);
    if (picture) {
        p = new PictureProperties(picture);
        connect(p, SIGNAL(applyEffects(int)), this, SLOT(slotApplyEffects(int)));
    }

    // text properties (dialog and connections)
    TextContent * text = dynamic_cast<TextContent *>(content);
    if (text) {
        p = new TextProperties(text);
    }

    // common properties
    if (p) {
        m_properties.append(p);
        addItem(p);
        connect(p, SIGNAL(closed()), this, SLOT(slotDeleteProperties()));
        connect(p, SIGNAL(applyLooks(quint32,bool)), this, SLOT(slotApplyLooks(quint32,bool)));
        p->show();
        p->setPos(scenePoint - QPoint(10, 10));
        p->keepInBoundaries(sceneRect().toRect());
    }
}

void Desk::slotBackgroundContent()
{
    AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
    if (!content)
        return;

    // re-show previous background
    if (m_backContent)
        m_backContent->show();

    // hide content item
    m_backContent = content;
    m_backContent->hide();
    m_backCache = QPixmap();
    update();
}

void Desk::slotStackContent(int op)
{
    AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
    if (!content || m_content.size() < 2)
        return;
    int size = m_content.size();
    int index = m_content.indexOf(content);

    // find out insertion indexes over the stacked items
    QList<QGraphicsItem *> stackedItems = items(content->sceneBoundingRect(), Qt::IntersectsItemShape);
    int prevIndex = 0;
    int nextIndex = size - 1;
    foreach (QGraphicsItem * item, stackedItems) {
        // operate only on different Content
        AbstractContent * c = dynamic_cast<AbstractContent *>(item);
        if (!c || c == content)
            continue;

        // refine previous/next indexes (close to 'index')
        int cIdx = m_content.indexOf(c);
        if (cIdx < nextIndex && cIdx > index)
            nextIndex = cIdx;
        else if (cIdx > prevIndex && cIdx < index)
            prevIndex = cIdx;
    }

    // move items
    switch (op) {
        case 1: // front
            m_content.append(m_content.takeAt(index));
            break;
        case 2: // raise
            if (index >= size - 1)
                return;
            m_content.insert(nextIndex, m_content.takeAt(index));
            break;
        case 3: // lower
            if (index <= 0)
                return;
            m_content.insert(prevIndex, m_content.takeAt(index));
            break;
        case 4: // back
            m_content.prepend(m_content.takeAt(index));
            break;
    }

    // reassign z-levels
    int z = 1;
    foreach (AbstractContent * content, m_content)
        content->setZValue(z++);
}

void Desk::slotDeleteContent()
{
    AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
    if (!content)
        return;

    // unset background if deleting its content
    if (m_backContent == content) {
        m_backContent = 0;
        m_backCache = QPixmap();
        update();
    }

    // remove property if deleting its content
    QList<AbstractProperties *>::iterator pIt = m_properties.begin();
    while (pIt != m_properties.end()) {
        AbstractProperties * pp = *pIt;
        if (pp->content() == content) {
            delete pp;
            pIt = m_properties.erase(pIt);
        } else
            ++pIt;
    }

    // unlink content from lists, myself(the Scene) and memory
    m_content.removeAll(content);
    removeItem(content);
    content->deleteLater();
}

void Desk::slotDeleteProperties()
{
    AbstractProperties * properties = dynamic_cast<AbstractProperties *>(sender());
    if (!properties)
        return;

    // unlink picture properties from lists, myself(the Scene) and memory
    m_properties.removeAll(properties);
    removeItem(properties);
    properties->deleteLater();
}

void Desk::slotApplyLooks(quint32 frameClass, bool mirrored)
{
    foreach (AbstractContent * content, m_content) {
        content->setFrame(FrameFactory::createFrame(frameClass));
        content->setMirrorEnabled(mirrored);
    }
}

void Desk::slotApplyEffects(int effectClass)
{
    foreach (AbstractContent * content, m_content) {
        PictureContent * picture = dynamic_cast<PictureContent *>(content);
        if (picture)
            picture->setEffect(effectClass);
    }
}

void Desk::slotSetTopBarEnabled(bool enabled)
{
    if (enabled == m_topBarEnabled)
        return;
    m_topBarEnabled = enabled;
    m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
    update();
}

void Desk::slotSetBottomBarEnabled(bool enabled)
{
    if (enabled == m_bottomBarEnabled)
        return;
    m_bottomBarEnabled = enabled;
    m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
    update();
}

void Desk::slotSetTitle()
{
    bool ok = false;
    QString title = QInputDialog::getText(0, tr("Title"), tr("Insert the title"), QLineEdit::Normal, m_titleText, &ok);
    if (ok)
        setTitleText(title);
}

void Desk::slotClearTitle()
{
    setTitleText(QString());
}

void Desk::slotTitleColorChanged()
{
    update(0, 0, m_size.width(), 50);
}

void Desk::slotForeColorChanged()
{
    update(0, 0, m_size.width(), 50);
    update(0, m_size.height() - 50, m_size.width(), 50);
}

void Desk::slotGradColorChanged()
{
    update();
}

void Desk::slotCloseHelp()
{
    m_helpItem->deleteLater();
    m_helpItem = 0;
    foreach (HighlightItem * highlight, m_highlightItems)
        highlight->deleteAfterAnimation();
    m_highlightItems.clear();
}
