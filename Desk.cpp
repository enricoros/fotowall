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
#include "frames/FrameFactory.h"
#include "items/ColorPickerItem.h"
#include "items/HelpItem.h"
#include "items/HighlightItem.h"
#include "items/PictureContent.h"
#include "items/PictureConfig.h"
#include "items/TextContent.h"
#include "items/TextConfig.h"
#include "items/VideoContent.h"
#include "items/WebContentSelectorItem.h"
#include "CropingDialog.h"
#include "FlickrInterface.h"
#include "RenderOpts.h"
#include <QAbstractTextDocumentLayout>
#include <QFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QImageReader>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>

#define COLORPICKER_W 200
#define COLORPICKER_H 150

Desk::Desk(QObject * parent)
    : QGraphicsScene(parent)
    , m_networkAccessManager(0)
    , m_helpItem(0)
    , m_backContent(0)
    , m_topBarEnabled(false)
    , m_bottomBarEnabled(false)
    , m_backGradientEnabled(true)
    , m_projectMode(ModeNormal)
    , m_webContentSelector(0)
    , m_forceFieldTimer(0)
{
    // create colorpickers
    m_titleColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_titleColorPicker->setColor(Qt::red);
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
    m_foreColorPicker->setVisible(false);
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

    // hooks
    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

#if 0
    // crazy background stuff
    #define RP QPointF(-400 + qrand() % 2000, -300 + qrand() % 1500)
    for ( int i = 0; i < 100; i++ ) {
        QGraphicsPathItem * p = new QGraphicsPathItem();
        addItem(p);
        p->show();
        p->setPen(QPen(QColor::fromHsv(qrand() % 20, 128 + qrand() % 127, qrand() % 255)));
        QPainterPath path(RP);
        path.cubicTo(RP, RP, RP);
        p->setPath(path);
    }
#endif
}

Desk::~Desk()
{
    delete m_forceFieldTimer;
    qDeleteAll(m_highlightItems);
    delete m_helpItem;
    delete m_titleColorPicker;
    delete m_foreColorPicker;
    delete m_grad1ColorPicker;
    delete m_grad2ColorPicker;
    qDeleteAll(m_content);
    m_content.clear();
    m_backContent = 0;
    delete m_networkAccessManager;
}

/// Add Content
static QPoint nearCenter(const QRectF & rect)
{
    return rect.center().toPoint() + QPoint(2 - (qrand() % 5), 2 - (qrand() % 5));
}

void Desk::addPictures(const QStringList & fileNames)
{
    QPoint pos = nearCenter(sceneRect());
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

void Desk::addTextContent()
{
    createText(nearCenter(sceneRect()));
}

void Desk::addVideoContent(int input)
{
    createVideo(input, nearCenter(sceneRect()));
}


/// Selectors
void Desk::setWebContentSelectorVisible(bool visible)
{
    if (!visible && m_webContentSelector) {
        removeItem(m_webContentSelector);
        m_webContentSelector->deleteLater();
        m_webContentSelector = 0;
    }
    if (visible && !m_webContentSelector) {
        if (!m_networkAccessManager)
            m_networkAccessManager = new QNetworkAccessManager(this);
        m_webContentSelector = new WebContentSelectorItem(m_networkAccessManager);
        m_webContentSelector->setPos(20, -8);
        addItem(m_webContentSelector);
    }
}

bool Desk::webContentSelectorVisible() const
{
    return m_webContentSelector;
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
    foreach (AbstractConfig * config, m_configs)
        config->keepInBoundaries(m_rect.toRect());

    // change my rect
    setSceneRect(m_rect);
}

/// Item Interaction
void Desk::selectAllContent(bool selected)
{
    foreach (AbstractContent * content, m_content)
        content->setSelected(selected);
}

/// Arrangement
void Desk::setForceFieldEnabled(bool enabled)
{
    if (enabled && !m_forceFieldTimer) {
        m_forceFieldTimer = new QTimer(this);
        connect(m_forceFieldTimer, SIGNAL(timeout()), this, SLOT(slotApplyForce()));
        m_forceFieldTimer->start(10);
        m_forceFieldTime.start();
    }

    if (!enabled && m_forceFieldTimer) {
        delete m_forceFieldTimer;
        m_forceFieldTimer = 0;
    }
}

bool Desk::forceFieldEnabled() const
{
    return m_forceFieldTimer;
}

/// Decorations
void Desk::setBackMode(int mode)
{
    // 1: background gradient / 2: transparent
    bool enableGradient = mode == 1;
    m_backGradientEnabled = enableGradient;
    m_grad1ColorPicker->setVisible(enableGradient);
    m_grad2ColorPicker->setVisible(enableGradient);
    if (enableGradient)
        blinkBackGradients();
    update();

    // 3: restore picture
    if (mode != 3 && m_backContent)
        setBackContent(0);

    // notify the change
    emit backModeChanged();
}

int Desk::backMode() const
{
    return m_backContent ? 3 : m_backGradientEnabled ? 1 : 2;
}

void Desk::setTopBarEnabled(bool enabled)
{
    if (enabled == m_topBarEnabled)
        return;
    m_topBarEnabled = enabled;
    m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
    update();
}

bool Desk::topBarEnabled() const
{
    return m_topBarEnabled;
}

void Desk::setBottomBarEnabled(bool enabled)
{
    if (enabled == m_bottomBarEnabled)
        return;
    m_bottomBarEnabled = enabled;
    m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
    update();
}

bool Desk::bottomBarEnabled() const
{
    return m_bottomBarEnabled;
}

void Desk::setTitleText(const QString & text)
{
    m_titleText = text;
    m_titleColorPicker->setVisible(!text.isEmpty());
    update(0, 0, m_size.width(), 50);
}

QString Desk::titleText() const
{
    return m_titleText;
}

/// Misc: save, restore, help...
#define HIGHLIGHT(x, y, del) \
    { \
        HighlightItem * highlight = new HighlightItem(); \
        if (!del) m_highlightItems.append(highlight); \
        else highlight->deleteAfterAnimation(); \
        addItem(highlight); \
        highlight->setZValue(10000); \
        highlight->setPosF(x, y); \
        highlight->show(); \
    }

void Desk::showIntroduction()
{
    if (m_helpItem)
        return;

    // help item
    m_helpItem = new HelpItem();
    connect(m_helpItem, SIGNAL(closeMe()), this, SLOT(slotCloseIntroduction()));
    addItem(m_helpItem);
    m_helpItem->setZValue(10001);
    m_helpItem->setPos(sceneRect().center().toPoint());
    m_helpItem->show();

    // blink items
    if (m_topBarEnabled || m_bottomBarEnabled)
        HIGHLIGHT(0.0, 0.0, false);
    if (!m_titleText.isEmpty())
        HIGHLIGHT(0.5, 0.0, false);
    if (m_backGradientEnabled) {
        HIGHLIGHT(1.0, 0.0, false);
        HIGHLIGHT(1.0, 1.0, false);
    }
}

void Desk::blinkBackGradients()
{
    HIGHLIGHT(1.0, 0.0, true);
    HIGHLIGHT(1.0, 1.0, true);
}

/// Modes
Desk::Mode Desk::projectMode() const
{
    return m_projectMode;
}

void Desk::setProjectMode(Mode mode)
{
    if (m_projectMode != mode) {
        m_projectMode = mode;
        switch (mode) {
            case ModeDVD:
                setDVDMarkers();
                break;
            default:
                clearMarkers();
                break;
        }
    }
}

void Desk::toXml(QDomElement & de) const
{
    QDomDocument doc = de.ownerDocument();

    // save Title
    QDomElement titleElement = doc.createElement("title");
    de.appendChild(titleElement);
    QDomText titleText = doc.createTextNode(m_titleText);
    titleElement.appendChild(titleText);

    // save background Colors
    QColor color;
    QString r, g, b;
    QDomElement domElement, topColor, bottomColor,
                redElement = doc.createElement("red"),
                greenElement = doc.createElement("green"),
                blueElement = doc.createElement("blue"),
                redElement2 = doc.createElement("red"),
                greenElement2 = doc.createElement("green"),
                blueElement2 = doc.createElement("blue"),
                rElement = doc.createElement("red"),
                gElement = doc.createElement("green"),
                bElement = doc.createElement("blue"),
                rElement2 = doc.createElement("red"),
                gElement2 = doc.createElement("green"),
                bElement2 = doc.createElement("blue");
    domElement = doc.createElement("background-color");

    topColor = doc.createElement("top");
    color = m_grad1ColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    redElement.appendChild(doc.createTextNode(r));
    greenElement.appendChild(doc.createTextNode(g));
    blueElement.appendChild(doc.createTextNode(b));
    topColor.appendChild(redElement); topColor.appendChild(greenElement); topColor.appendChild(blueElement);
    domElement.appendChild(topColor);

    bottomColor = doc.createElement("bottom");
    color = m_grad2ColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    redElement2.appendChild(doc.createTextNode(r));
    greenElement2.appendChild(doc.createTextNode(g));
    blueElement2.appendChild(doc.createTextNode(b));
    bottomColor.appendChild(redElement2); bottomColor.appendChild(greenElement2); bottomColor.appendChild(blueElement2);
    domElement.appendChild(bottomColor);

    de.appendChild(domElement);

    QDomElement titleColor = doc.createElement("title-color");
    color = m_titleColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    rElement.appendChild(doc.createTextNode(r));
    gElement.appendChild(doc.createTextNode(g));
    bElement.appendChild(doc.createTextNode(b));
    titleColor.appendChild(rElement); titleColor.appendChild(gElement); titleColor.appendChild(bElement);
    de.appendChild(titleColor);

    QDomElement foreColor = doc.createElement("foreground-color");
    color = m_foreColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    rElement2.appendChild(doc.createTextNode(r));
    gElement2.appendChild(doc.createTextNode(g));
    bElement2.appendChild(doc.createTextNode(b));
    foreColor.appendChild(rElement2); foreColor.appendChild(gElement2); foreColor.appendChild(bElement2);
    de.appendChild(foreColor);
}


void Desk::fromXml(QDomElement & de)
{
    setTitleText(de.firstChildElement("title").text());

    QDomElement domElement;
    int r, g, b;

    domElement = de.firstChildElement("background-color").firstChildElement("top");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_grad1ColorPicker->setColor(QColor(r, g, b));

    domElement = de.firstChildElement("background-color").firstChildElement("bottom");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_grad2ColorPicker->setColor(QColor(r, g, b));

    domElement = de.firstChildElement("title-color");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_titleColorPicker->setColor(QColor(r, g, b));

    domElement = de.firstChildElement("foreground-color");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_foreColorPicker->setColor(QColor(r, g, b));

    update();
}

void Desk::renderVisible(QPainter * painter, const QRectF & target, const QRectF & source, Qt::AspectRatioMode aspectRatioMode)
{
    clearSelection();
    foreach(QGraphicsItem *item, m_markerItems)
        item->hide();
    foreach(AbstractConfig *conf, m_configs)
        conf->hide();

    RenderOpts::HQRendering = true;
    QGraphicsScene::render(painter, target, source, aspectRatioMode);
    RenderOpts::HQRendering = false;

    foreach(AbstractConfig *conf, m_configs)
        conf->show();
    foreach(QGraphicsItem *item, m_markerItems)
        item->show();
}

QImage Desk::renderedImage(const QSize & iSize, Qt::AspectRatioMode aspectRatioMode)
{
    QImage result(iSize, QImage::Format_ARGB32);
    result.fill(0);

    QPainter painter(&result);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QSize targetSize = sceneRect().size().toSize();
    targetSize.scale(iSize, aspectRatioMode);
    int offsetX = (iSize.width() - targetSize.width()) / 2;
    int offsetY = (iSize.height() - targetSize.height()) / 2;

    QRect targetRect = QRect(offsetX, offsetY, targetSize.width(), targetSize.height());
    renderVisible(&painter, targetRect, sceneRect(), Qt::IgnoreAspectRatio);
    painter.end();

    return result;
}

bool Desk::printAsImage(int printerDpi, const QSize & pixelSize, bool landscape, Qt::AspectRatioMode aspectRatioMode)
{
    // setup printer
    QPrinter printer;
    printer.setResolution(printerDpi);
    printer.setPaperSize(QPrinter::A4);

    // configure printer via the print dialog
    QPrintDialog printDialog(&printer);
    if (printDialog.exec() != QDialog::Accepted)
        return false;

    // TODO: use different ratio modes?
    QImage image = renderedImage(pixelSize, aspectRatioMode);
    if (landscape) {
        // Print in landscape mode, so rotate
        QMatrix matrix;
        matrix.rotate(90);
        image = image.transformed(matrix);
    }

    // And then print
    QPainter paint(&printer);
    paint.drawImage(image.rect(), image);
    paint.end();
    return true;
}

/// Drag & Drop image files
void Desk::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children but accept it only for image files
    QGraphicsScene::dragEnterEvent(event);
    event->ignore();

    // skip bad mimes
    if (!event->mimeData())
        return;

    // check files drop
    if (event->mimeData()->hasUrls()) {

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

    // check content drop
    if (event->mimeData()->hasFormat("webselector/idx")) {
        event->accept();
        return;
    }
}

void Desk::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children
    event->ignore();
    QGraphicsScene::dragMoveEvent(event);

    // or continue accepting event for the Desk
    if (!event->isAccepted()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void Desk::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // handle by children
    event->ignore();
    QGraphicsScene::dropEvent(event);
    if (event->isAccepted())
        return;

    // handle as a Desk file drop event
    if (event->mimeData()->hasUrls()) {
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
        return;
    }

    // handle as a Desk content drop event
    if (event->mimeData()->hasFormat("webselector/idx") && m_webContentSelector) {

        // get the flickr interface
        FlickrInterface * flickr = m_webContentSelector->flickrInterface();
        if (!flickr)
            return;

        // download each picture
        QPoint insertPos = event->scenePos().toPoint();
        QStringList sIndexes = QString(event->mimeData()->data("webselector/idx")).split(",");
        foreach (const QString & sIndex, sIndexes) {
            int index = sIndex.toUInt();

            // get picture description
            QString url;
            QString title;
            int width = 0;
            int height = 0;
            if (!flickr->imageInfo(index, &url, &title, &width, &height))
                continue;

            // get the download
            QNetworkReply * reply = flickr->download(index);
            if (!reply)
                continue;

            // create PictureContent from network
            PictureContent * p = createPicture(insertPos);
            if (!p->loadFromNetwork(url, reply, title, width, height)) {
                m_content.removeAll(p);
                delete p;
            } else
                insertPos += QPoint(30, 30);
        }
        event->accept();
    }
}

void Desk::keyPressEvent(QKeyEvent * keyEvent)
{
    QGraphicsScene::keyPressEvent(keyEvent);
    if (!keyEvent->isAccepted() && keyEvent->key() == Qt::Key_Delete)
        slotDeleteContent();
}

void Desk::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    // first dispatch doubleclick to items
    mouseEvent->ignore();
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    // unset the background picture, if present
    setBackMode(m_backGradientEnabled ? 1 : 2);
}

void Desk::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
    // context menu on empty area
    //if (items(event->scenePos()).isEmpty()) {
    //}
    QGraphicsScene::contextMenuEvent(event);
}


/// Scene Background & Foreground
void Desk::drawBackground(QPainter * painter, const QRectF & rect)
{
    // draw content if set
    if (m_backContent) {
        // regenerate cache if needed
        QSize sceneSize = sceneRect().size().toSize();
        if (m_backCache.isNull() || m_backCache.size() != sceneSize)
            m_backCache = m_backContent->renderAsBackground(sceneSize, false);

        // paint cached background
        QRect targetRect = rect.toRect();
        if (m_backContent->contentOpaque())
            painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawPixmap(targetRect, m_backCache, targetRect);
        if (m_backContent->contentOpaque())
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        return;
    }

    // draw background gradient, if enabled
    if (m_backGradientEnabled) {
        QLinearGradient lg(m_rect.topLeft(), m_rect.bottomLeft());
        lg.setColorAt(0.0, m_grad1ColorPicker->color());
        lg.setColorAt(1.0, m_grad2ColorPicker->color());
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, lg);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
}

static void drawVerticalShadow(QPainter * painter, int width, int height)
{
    QLinearGradient lg( 0, 0, 0, height );
    lg.setColorAt( 0.0, QColor( 0, 0, 0, 64 ) );
    lg.setColorAt( 0.4, QColor( 0, 0, 0, 16 ) );
    lg.setColorAt( 0.7, QColor( 0, 0, 0, 5 ) );
    lg.setColorAt( 1.0, QColor( 0, 0, 0, 0 ) );
    painter->fillRect( 0, 0, width, height, lg );
}

void Desk::drawForeground(QPainter * painter, const QRectF & rect)
{
    // draw header/footer
    const int top = (int)rect.top();
    const int bottom = (int)rect.bottom();
    const int left = (int)rect.left();
    const int width = (int)(rect.width() + 1.0);
    if (m_topBarEnabled || m_bottomBarEnabled) {
        QColor hColor = m_foreColorPicker->color();
        hColor.setAlpha(128);
        if (m_topBarEnabled && top < 50)
            painter->fillRect(left, 0, width, 50, hColor);
        if (m_bottomBarEnabled && bottom >= m_size.height() - 50)
            painter->fillRect(left, m_size.height() - 50, width, 50, hColor);
    }

    // draw text
    if (!m_titleText.isEmpty()) {
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

    // draw top shadow (only on screen)
    if (!RenderOpts::HQRendering) {
        // the first time create the Shadow Tile
        static QPixmap shadowTile;
        if (shadowTile.isNull()) {
            shadowTile = QPixmap(64, 8);
            shadowTile.fill(Qt::transparent);
            QPainter shadowPainter(&shadowTile);
            drawVerticalShadow(&shadowPainter, 64, 8);
        }

        // blend the shadow tile
        if (top < 8)
            painter->drawTiledPixmap(left, 0, width, 8, shadowTile);
    }
}

void Desk::initContent(AbstractContent * content, const QPoint & pos)
{
    connect(content, SIGNAL(configureMe(const QPoint &)), this, SLOT(slotConfigureContent(const QPoint &)));
    connect(content, SIGNAL(backgroundMe()), this, SLOT(slotBackgroundContent()));
    connect(content, SIGNAL(changeStack(int)), this, SLOT(slotStackContent(int)));
    connect(content, SIGNAL(deleteItem()), this, SLOT(slotDeleteContent()));

    if (!pos.isNull())
        content->setPos(pos);
    content->setZValue(m_content.isEmpty() ? 1 : (m_content.last()->zValue() + 1));
    //content->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    content->show();

    m_content.append(content);
}

void Desk::setBackContent(AbstractContent * content)
{
    // skip if unchanged
    if (content == m_backContent)
        return;

    // re-show previous background
    if (m_backContent) {
        disconnect(m_backContent, SIGNAL(contentChanged()), this, SLOT(slotBackContentChanged()));
        m_backContent->show();
    }

    // hide content item
    m_backContent = content;
    if (m_backContent) {
        connect(m_backContent, SIGNAL(contentChanged()), this, SLOT(slotBackContentChanged()));
        m_backContent->hide();
    }

    // update GUI
    m_backCache = QPixmap();
    update();
    emit backModeChanged();
}

PictureContent * Desk::createPicture(const QPoint & pos)
{
    PictureContent * p = new PictureContent(this);
    initContent(p, pos);
    connect(p, SIGNAL(flipHorizontally()), this, SLOT(slotFlipHorizontally()));
    connect(p, SIGNAL(flipVertically()), this, SLOT(slotFlipVertically()));
    connect(p, SIGNAL(crop()), this, SLOT(slotCrop()));
    return p;
}

TextContent * Desk::createText(const QPoint & pos)
{
    TextContent * t = new TextContent(this);
    initContent(t, pos);
    return t;
}

VideoContent * Desk::createVideo(int input, const QPoint & pos)
{
    VideoContent * v = new VideoContent(input, this);
    initContent(v, pos);
    return v;
}

/// Markers
void Desk::setDVDMarkers()
{
    // Add informations items to show the back, front, and side position

    QGraphicsView * view = views().first();
    int faceW = 5.08 * view->logicalDpiX();
    int sideW = 0.67 * view->logicalDpiY();
    m_markerItems.push_back(addLine(faceW, 0, faceW, height()));
    m_markerItems.push_back(addLine(faceW+sideW, 0, faceW+sideW, height()));

    QGraphicsTextItem *textBack = addText(tr("Back"), QFont("", 18, -1, true));
    textBack->setPos( (faceW - textBack->document()->documentLayout()->documentSize().width())/2,
                    (height() - textBack->document()->documentLayout()->documentSize().height())/2 );
    m_markerItems.push_back(textBack);
    QGraphicsTextItem *textFront = addText(tr("Front"), QFont("", 18, -1, true));
    textFront->setPos( (faceW+sideW) + faceW/2 - textFront->document()->documentLayout()->documentSize().width()/2,
                    (height() - textFront->document()->documentLayout()->documentSize().height())/2 );
    m_markerItems.push_back(textFront);
}

void Desk::clearMarkers()
{
    // Remove the information items
    qDeleteAll(m_markerItems);
    m_markerItems.clear();
}

/// Slots
void Desk::slotSelectionChanged()
{
    // show the config widget if 1 AbstractContent is selected
    QList<QGraphicsItem *> selection = selectedItems();
    if (selection.size() == 1) {
        AbstractContent * content = dynamic_cast<AbstractContent *>(selection.first());
        if (content) {
            QWidget * pWidget = content->createPropertyWidget();
            if (pWidget)
                pWidget->setWindowTitle(tr("%1").arg(content->contentName().toUpper()));
            emit showPropertiesWidget(pWidget);
            return;
        }
    }

    // show a 'selection' properties widget
    if (selection.size() > 1) {
        QLabel * label = new QLabel(tr("%1 objects selected").arg(selection.size()));
        label->setWindowTitle(tr("SELECTION"));
        emit showPropertiesWidget(label);
        return;
    }

    // or don't show anything
    emit showPropertiesWidget(0);
}

void Desk::slotConfigureContent(const QPoint & scenePoint)
{
    // get the content and ensure it hasn't already a property window
    AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
    foreach (AbstractConfig * config, m_configs) {
        if (config->content() == content)
            return;
        // force only 1 property instance
        slotDeleteConfig(config);
    }
    AbstractConfig * p = 0;

    // picture config (dialog and connections)
    if (PictureContent * picture = dynamic_cast<PictureContent *>(content)) {
        p = new PictureConfig(picture);
        connect(p, SIGNAL(applyEffect(const PictureEffect &, bool)), this, SLOT(slotApplyEffect(const PictureEffect &, bool)));
    }

    // text config (dialog and connections)
    if (TextContent * text = dynamic_cast<TextContent *>(content))
        p = new TextConfig(text);

    // generic config
    if (!p)
        p = new AbstractConfig(content);

    // common links
    m_configs.append(p);
    addItem(p);
    connect(p, SIGNAL(applyLook(quint32,bool,bool)), this, SLOT(slotApplyLook(quint32,bool,bool)));
    p->show();
    p->setPos(scenePoint - QPoint(10, 10));
    p->keepInBoundaries(sceneRect().toRect());
    p->setFocus();
}

void Desk::slotBackgroundContent()
{
    setBackContent(dynamic_cast<AbstractContent *>(sender()));
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

static QList<AbstractContent *> content(const QList<QGraphicsItem *> & items) {
    QList<AbstractContent *> contentList;
    foreach (QGraphicsItem * item, items) {
        AbstractContent * c = dynamic_cast<AbstractContent *>(item);
        if (c)
            contentList.append(c);
    }
    return contentList;
}

void Desk::slotDeleteContent()
{
    QList<AbstractContent *> selectedContent = content(selectedItems());
    AbstractContent * senderContent = dynamic_cast<AbstractContent *>(sender());
    if (senderContent && !selectedContent.contains(senderContent)) {
        selectedContent.clear();
        selectedContent.append(senderContent);
    }
    if (selectedContent.size() > 1)
        if (QMessageBox::question(0, tr("Delete content"), tr("All the %1 selected content will be deleted, do you want to continue ?").arg(selectedContent.size()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;

    foreach (AbstractContent * content, selectedContent) {

        // unset background if deleting its content
        if (m_backContent == content)
            setBackContent(0);

        // remove related property if deleting its content
        foreach (AbstractConfig * config, m_configs) {
            if (config->content() == content) {
                slotDeleteConfig(config);
                break;
            }
        }

        // unlink content from lists, myself(the Scene) and memory
        m_content.removeAll(content);
        content->dispose();
    }
}

void Desk::slotDeleteConfig(AbstractConfig * config)
{
    m_configs.removeAll(config);
    config->dispose();
}

void Desk::slotApplyLook(quint32 frameClass, bool mirrored, bool all)
{
    QList<AbstractContent *> selectedContent = content(selectedItems());
    foreach (AbstractContent * content, m_content) {
        if (all || selectedContent.contains(content)) {
            if (content->frameClass() != frameClass)
                content->setFrame(FrameFactory::createFrame(frameClass));
            content->setMirrorEnabled(mirrored);
        }
    }
}

void Desk::slotApplyEffect(const PictureEffect & effect, bool all)
{
    QList<AbstractContent *> selectedContent = content(selectedItems());
    foreach (AbstractContent * content, m_content) {
        PictureContent * picture = dynamic_cast<PictureContent *>(content);
        if (!picture)
            continue;

        if (all || selectedContent.contains(content))
            picture->addEffect(effect);
    }
}

void Desk::slotCrop()
{
    QList<AbstractContent *> selectedContent = content(selectedItems());
    foreach (AbstractContent * content, selectedContent) {
        PictureContent * picture = dynamic_cast<PictureContent *>(content);
        if (!picture)
            continue;
        CPixmap photo = picture->getPhoto();
        CropingDialog dial(&photo);
        if(dial.exec() == QDialog::Accepted) {
            QRect cropingRect = dial.getCropingRect();
            if(!cropingRect.isNull()) {
                picture->addEffect(PictureEffect(PictureEffect::Crop, 0, dial.getCropingRect()));
            }
        }
    }
}

void Desk::slotFlipHorizontally()
{
    QList<AbstractContent *> selectedContent = content(selectedItems());
    foreach (AbstractContent * content, selectedContent) {
        PictureContent * picture = dynamic_cast<PictureContent *>(content);
        if (!picture)
            continue;
        picture->addEffect(PictureEffect::FlipH);
    }
}

void Desk::slotFlipVertically()
{
    QList<AbstractContent *> selectedContent = content(selectedItems());
    foreach (AbstractContent * content, selectedContent) {
        PictureContent * picture = dynamic_cast<PictureContent *>(content);
        if (!picture)
            continue;
        picture->addEffect(PictureEffect::FlipV);
    }
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

void Desk::slotBackContentChanged()
{
    m_backCache = QPixmap();
    update();
}

void Desk::slotCloseIntroduction()
{
    m_helpItem->dispose();
    m_helpItem = 0;
    foreach (HighlightItem * highlight, m_highlightItems)
        highlight->deleteAfterAnimation();
    m_highlightItems.clear();
}

void Desk::slotApplyForce()
{
    // initial consts
    const QRectF sRect = sceneRect();
    if (sRect.width() < 10 || sRect.height() < 10)
        return;
    const qreal W = sRect.width();
    const qreal H = sRect.height();
    const qreal dT = 4.0 * qBound((qreal)0.001, (qreal)m_forceFieldTime.restart() / 1000.0, (qreal)0.10);

    // pass 0
    QList<AbstractContent *>::iterator it1, it2, end = m_content.end();
    for (it1 = m_content.begin(); it1 != end; ++it1) {
        AbstractContent * t = *it1;
        t->vPos = Vector2(t->pos().x(), t->pos().y());
        double fx = W / (t->vPos.x() - sRect.left() + 10.0) + W / (t->vPos.x() - sRect.right() - 10.0);
        double fy = H / (t->vPos.y() - sRect.top() + 10.0) + H / (t->vPos.y() - sRect.bottom() - 10.0);
        t->vForce = Vector2(fx, fy);
    }

    // pass 1: item-vs-item force
    for (it1 = m_content.begin(); it1 != end; ++it1) {
        for (it2 = m_content.begin(); it2 != end; ++it2) {
            AbstractContent * t = *it1;
            AbstractContent * s = *it2;
            Vector2 r = t->vPos - s->vPos;
            double mod = r.module();
            if (mod > 0.707) {
                r *= s->boundingRect().width() / (mod * mod);
                t->vForce += r;
            }
        }
    }

    // pass 2: apply force
    for (it1 = m_content.begin(); it1 != end; ++it1) {
        AbstractContent * t = *it1;
        if (t->isSelected())
            continue;

        Vector2 vStart = t->vVel;

        // add friction
        t->vForce += -0.2 * t->vVel;

        t->vVel += t->vForce * dT;
        t->vPos += (vStart + t->vVel) * dT / 2.0;
        t->setPos(t->vPos.x(), t->vPos.y());
    }
}
