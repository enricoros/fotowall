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

#include "Canvas.h"

#include "Frames/FrameFactory.h"
#include "Shared/AbstractPictureService.h"
#include "Shared/ColorPickerItem.h"
#include "CanvasModeInfo.h"
#include "CanvasViewContent.h"
#include "HelpItem.h"
#include "HighlightItem.h"
#include "PictureContent.h"
#include "PictureConfig.h"
#include "PictureSearchItem.h"
#include "SelectionProperties.h"
#include "TextContent.h"
#include "TextConfig.h"
#include "WebcamContent.h"
#include "WordCloudContent.h"
#include "Shared/RenderOpts.h"

#include <QAbstractTextDocumentLayout>
#include <QFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QImageReader>
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

Canvas::Canvas(const QSize & initialSize, QObject * parent)
    : AbstractScene(parent)
    , m_modeInfo(new CanvasModeInfo)
    , m_networkAccessManager(0)
    , m_helpItem(0)
    , m_backContent(0)
    , m_topBarEnabled(false)
    , m_bottomBarEnabled(false)
    , m_backGradientEnabled(true)
    , m_backContentRatio(Qt::KeepAspectRatioByExpanding)
    , m_pictureSearch(0)
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

    // selection self-hook
    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    // precreate background tile
    m_backTile = QPixmap(100, 100);
    m_backTile.fill(Qt::lightGray);
    QPainter tilePainter(&m_backTile);
    tilePainter.fillRect(0, 0, 50, 50, Qt::darkGray);
    tilePainter.fillRect(50, 50, 50, 50, Qt::darkGray);
    tilePainter.end();

    // set the initial size of the canvas, don't let it grow automatically
    resize(initialSize);

    // crazy background stuff
#if 0
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

Canvas::~Canvas()
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
    delete m_modeInfo;
}

/// Add Content
static QPoint nearCenter(const QRectF & rect)
{
    return rect.center().toPoint() + QPoint(2 - (qrand() % 5), 2 - (qrand() % 5));
}

void Canvas::addCanvasViewContent(const QStringList & fileNames)
{
    int offset = -30 * fileNames.size() / 2;
    QPoint pos = nearCenter(sceneRect()) + QPoint(offset, offset);
    foreach (const QString & localFile, fileNames) {
        if (!QFile::exists(localFile))
            continue;

        // create picture and load the file
        CanvasViewContent * d = createCanvasView(pos);
        if (!d->loadCanvas(localFile, true, true)) {
            m_content.removeAll(d);
            delete d;
        } else
            pos += QPoint(30, 30);
    }
}

void Canvas::addPictureContent(const QStringList & fileNames)
{
    int offset = -30 * fileNames.size() / 2;
    QPoint pos = nearCenter(sceneRect()) + QPoint(offset, offset);
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

void Canvas::addTextContent()
{
    createText(nearCenter(sceneRect()));
}

void Canvas::addWebcamContent(int input)
{
    createWebcam(input, nearCenter(sceneRect()));
}

#include "App/App.h"
#include "App/MainWindow.h"
void Canvas::addWordCloudContent()
{
    WordCloudContent * wcc = createWordCloud(nearCenter(sceneRect()));
    App::mainWindow->editWordcloud(wcc->cloud());
}

void Canvas::resize(const QSize & size)
{
    // handle the fixed resizes
    if (m_modeInfo->fixedSize()) {
        QSize fixedSize = m_modeInfo->fixedScreenPixels();
        if (size != fixedSize)
            AbstractScene::resize(fixedSize);
        return;
    }
    // handle the normal resizes
    AbstractScene::resize(size);
}

void Canvas::resizeEvent(QResizeEvent * /*event*/)
{
    // relayout contents
    m_titleColorPicker->setPos((sceneWidth() - COLORPICKER_W) / 2.0, 10);
    m_grad1ColorPicker->setPos(sceneWidth() - COLORPICKER_W, 0);
    m_grad2ColorPicker->setPos(sceneWidth() - COLORPICKER_W, sceneHeight() - COLORPICKER_H);
    if (m_helpItem)
        m_helpItem->setPos(sceneCenter().toPoint());
    foreach (HighlightItem * highlight, m_highlightItems)
        highlight->reposition(sceneRect());

    // ensure visibility
    foreach (AbstractContent * content, m_content)
        content->ensureVisible(sceneRect());
    foreach (AbstractConfig * config, m_configs)
        config->keepInBoundaries(sceneRect());
}

/// Item Interaction
void Canvas::selectAllContent(bool selected)
{
    foreach (AbstractContent * content, m_content)
        content->setSelected(selected);
}

/// Picture Search
void Canvas::setSearchPicturesVisible(bool visible)
{
    // destroy if needed
    if (!visible && m_pictureSearch) {
        removeItem(m_pictureSearch);
        m_pictureSearch->deleteLater();
        m_pictureSearch = 0;
        return;
    }

    // create if needed
    if (visible && !m_pictureSearch) {
        if (!m_networkAccessManager)
            m_networkAccessManager = new QNetworkAccessManager(this);
        m_pictureSearch = new PictureSearchItem(m_networkAccessManager);
        m_pictureSearch->setZValue(999999);
        //m_pictureSearch->setPos(20, 0);
        addItem(m_pictureSearch);
        return;
    }
}

bool Canvas::searchPicturesVisible() const
{
    return m_pictureSearch;
}

/// Arrangement
void Canvas::setForceFieldEnabled(bool enabled)
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

bool Canvas::forceFieldEnabled() const
{
    return m_forceFieldTimer;
}

/// Decorations
void Canvas::setBackMode(int mode)
{
    // 1: none / 2: background gradient
    bool enableGradient = mode == 2;
    m_backGradientEnabled = enableGradient;
    m_grad1ColorPicker->setVisible(enableGradient);
    m_grad2ColorPicker->setVisible(enableGradient);
    if (enableGradient)
        blinkBackGradients();
    update();

    // 3: restore picture if changing from mode 3
    if (mode != 3 && m_backContent)
        setBackContent(0);

    // notify the change
    emit backModeChanged();
}

int Canvas::backMode() const
{
    return m_backContent ? 3 : m_backGradientEnabled ? 2 : 1;
}

void Canvas::setBackContentRatio(Qt::AspectRatioMode mode)
{
    if (m_backContentRatio != mode) {
        m_backContentRatio = mode;
        m_backCache = QPixmap();
        update();
    }
}

Qt::AspectRatioMode Canvas::backContentRatio() const
{
    return m_backContentRatio;
}

void Canvas::setTopBarEnabled(bool enabled)
{
    if (enabled == m_topBarEnabled)
        return;
    m_topBarEnabled = enabled;
    m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
    update();
}

bool Canvas::topBarEnabled() const
{
    return m_topBarEnabled;
}

void Canvas::setBottomBarEnabled(bool enabled)
{
    if (enabled == m_bottomBarEnabled)
        return;
    m_bottomBarEnabled = enabled;
    m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
    update();
}

bool Canvas::bottomBarEnabled() const
{
    return m_bottomBarEnabled;
}

void Canvas::setTitleText(const QString & text)
{
    m_titleText = text;
    m_titleColorPicker->setVisible(!text.isEmpty());
    update(0, 0, sceneWidth(), 50);
}

QString Canvas::titleText() const
{
    return m_titleText;
}

void Canvas::setCDMarkers()
{
    clearMarkers();
    QPen outerPen(Qt::black, 1, Qt::DashLine);
    QPen innerPen(Qt::lightGray, 1, Qt::DashLine);
    QSize screenPixels = m_modeInfo->fixedScreenPixels();
    QPointF screenDpi = m_modeInfo->screenDpi();

    float xDiameter = 4.75 * screenDpi.x();
    float yDiameter = 4.75 * screenDpi.y();
    QGraphicsEllipseItem * ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter, yDiameter);
    ellipse->setZValue(-1);
    ellipse->setPen(outerPen);
    m_markerItems.push_back(ellipse);

    xDiameter = 0.59 * screenDpi.x();
    yDiameter = 0.59 * screenDpi.y();
    ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter, yDiameter);
    ellipse->setZValue(-1);
    ellipse->setPen(innerPen);
    m_markerItems.push_back(ellipse);

    xDiameter = 1.34 * screenDpi.x();
    yDiameter = 1.34 * screenDpi.y();
    ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter, yDiameter);
    ellipse->setZValue(-1);
    ellipse->setPen(innerPen);
    m_markerItems.push_back(ellipse);

    xDiameter = 1.73 * screenDpi.x();
    yDiameter = 1.73 * screenDpi.y();
    ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter, yDiameter);
    ellipse->setZValue(-1);
    ellipse->setPen(innerPen);
    m_markerItems.push_back(ellipse);
}

void Canvas::setDVDMarkers()
{
    // Add informations items to show the back, front, and side position
    clearMarkers();
    QPen linePen(Qt::black, 1, Qt::DashLine);
    QPointF screenDpi = m_modeInfo->screenDpi();
    int faceW = 5.08 * screenDpi.x();
    int sideW = 0.67 * screenDpi.y();
    int height = m_modeInfo->fixedScreenPixels().height();
    QGraphicsLineItem * line = addLine((qreal)faceW + 0.5, 0, (qreal)faceW + 0.5, height);
    line->setPen(linePen);
    line->setZValue(-1);
    m_markerItems.push_back(line);
    line = addLine((qreal)(faceW + sideW) + 0.5, 0, (qreal)(faceW + sideW) + 0.5, height),
    line->setPen(linePen);
    line->setZValue(-1);
    m_markerItems.push_back(line);

    QFont markerFont;
    markerFont.setPointSize(18);
    markerFont.setItalic(true);
    QGraphicsTextItem *textBack = addText(tr("Back"), markerFont);
    textBack->setTransform(QTransform(-1, 0, 0, 0, 1, 0, 0, 0, 1));
    textBack->setPos((faceW + textBack->document()->documentLayout()->documentSize().width()) / 2,
                     (height - textBack->document()->documentLayout()->documentSize().height()) / 2);
    textBack->setZValue(-1);
    m_markerItems.push_back(textBack);
    QGraphicsTextItem *textFront = addText(tr("Front"), markerFont);
    textFront->setPos((faceW+sideW) + faceW/2 - textFront->document()->documentLayout()->documentSize().width() / 2,
                      (height - textFront->document()->documentLayout()->documentSize().height()) / 2);
    textFront->setZValue(-1);
    m_markerItems.push_back(textFront);
}

void Canvas::clearMarkers()
{
    // Remove the information items
    qDeleteAll(m_markerItems);
    m_markerItems.clear();
}

/// Misc: save, restore, help...
bool Canvas::pendingChanges() const
{
    return !m_content.isEmpty();
}

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

void Canvas::showIntroduction()
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

void Canvas::blinkBackGradients()
{
    HIGHLIGHT(1.0, 0.0, true);
    HIGHLIGHT(1.0, 1.0, true);
}

/// Modes
CanvasModeInfo * Canvas::modeInfo() const
{
    return m_modeInfo;
}
/*
void Canvas::setModeInfo(CanvasModeInfo * modeInfo)
{
    // set the new modeinfo
    delete m_modeInfo;
    m_modeInfo = modeInfo;

    // apply the fixed size (if defined)
    resize(sceneSize());

    // notify listeners (if any!) about the change
    emit refreshCanvas();
}
*/
void Canvas::toXml(QDomElement & canvasElement) const
{
    QDomDocument doc = canvasElement.ownerDocument();

    // MODEINFO
    {
        QDomElement modeElement = doc.createElement("mode");
         canvasElement.appendChild(modeElement);

        // save modeInfo
        m_modeInfo->toXml(modeElement);
    }

    // BACKGROUND
    {
        QDomElement backgroundElement = doc.createElement("background");
         canvasElement.appendChild(backgroundElement);

        // save Title
        QDomElement titleElement = doc.createElement("title");
         backgroundElement.appendChild(titleElement);
         titleElement.appendChild(doc.createTextNode(m_titleText));

        // save background Colors
        QColor color;
        QDomElement  redElement = doc.createElement("red"),
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
        QDomElement bgColorElement = doc.createElement("background-color");
        backgroundElement.appendChild(bgColorElement);

        QDomElement topColor = doc.createElement("top");
         color = m_grad1ColorPicker->color();
         redElement.appendChild(doc.createTextNode(QString::number(color.red())));
         greenElement.appendChild(doc.createTextNode(QString::number(color.green())));
         blueElement.appendChild(doc.createTextNode(QString::number(color.blue())));
         topColor.appendChild(redElement); topColor.appendChild(greenElement); topColor.appendChild(blueElement);
         bgColorElement.appendChild(topColor);

        QDomElement bottomColor = doc.createElement("bottom");
         color = m_grad2ColorPicker->color();
         redElement2.appendChild(doc.createTextNode(QString::number(color.red())));
         greenElement2.appendChild(doc.createTextNode(QString::number(color.green())));
         blueElement2.appendChild(doc.createTextNode(QString::number(color.blue())));
         bottomColor.appendChild(redElement2); bottomColor.appendChild(greenElement2); bottomColor.appendChild(blueElement2);
         bgColorElement.appendChild(bottomColor);

        QDomElement titleColor = doc.createElement("title-color");
         color = m_titleColorPicker->color();
         rElement.appendChild(doc.createTextNode(QString::number(color.red())));
         gElement.appendChild(doc.createTextNode(QString::number(color.green())));
         bElement.appendChild(doc.createTextNode(QString::number(color.blue())));
         titleColor.appendChild(rElement); titleColor.appendChild(gElement); titleColor.appendChild(bElement);
         backgroundElement.appendChild(titleColor);

        QDomElement foreColor = doc.createElement("foreground-color");
         color = m_foreColorPicker->color();
         rElement2.appendChild(doc.createTextNode(QString::number(color.red())));
         gElement2.appendChild(doc.createTextNode(QString::number(color.green())));
         bElement2.appendChild(doc.createTextNode(QString::number(color.blue())));
         foreColor.appendChild(rElement2); foreColor.appendChild(gElement2); foreColor.appendChild(bElement2);
         backgroundElement.appendChild(foreColor);

        // save back content aspect
        QDomElement backRatioElement = doc.createElement("back-properties");
         backRatioElement.setAttribute("ratio", (int)m_backContentRatio);
         backgroundElement.appendChild(backRatioElement);
    }

    // CONTENT
    {
        QDomElement contentElement = doc.createElement("content");
         canvasElement.appendChild(contentElement);

        // save contents
        foreach (const AbstractContent * content, m_content) {
            // save content
            QDomElement cEl = doc.createElement("dummy-renamed-element");
            contentElement.appendChild(cEl);
            content->toXml(cEl);

            // add a flag to the background element
            if (m_backContent == content) {
                QDomElement bgEl = doc.createElement("set-as-background");
                cEl.appendChild(bgEl);
            }
        }
    }
}

void Canvas::fromXml(QDomElement & canvasElement)
{
    // clear contents
    while (!m_content.isEmpty())
        deleteContent(m_content.first());
    while (!m_configs.isEmpty())
        deleteConfig(m_configs.first());

    // MODEINFO
    {
        // find the 'mode' element
        QDomElement modeElement = canvasElement.firstChildElement("mode");
        if (!modeElement.isElement()) // 'Format 1'
            modeElement = canvasElement.firstChildElement("project").firstChildElement("mode");

        // read and apply properties
        if (modeElement.isElement())
            m_modeInfo->fromXml(modeElement);
    }

    // BACKGROUND
    {
        // find the 'background' element
        QDomElement backgroundElement = canvasElement.firstChildElement("background");
        if (!backgroundElement.isElement()) // 'Format 1'
            backgroundElement = canvasElement.firstChildElement("desk");

        // read and apply properties
        if (backgroundElement.isElement()) {
            // title text
            setTitleText(backgroundElement.firstChildElement("title").text());

            // colors
            QDomElement domElement = backgroundElement.firstChildElement("background-color").firstChildElement("top");
             int r = domElement.firstChildElement("red").text().toInt();
             int g = domElement.firstChildElement("green").text().toInt();
             int b = domElement.firstChildElement("blue").text().toInt();
             m_grad1ColorPicker->setColor(QColor(r, g, b));
            domElement = backgroundElement.firstChildElement("background-color").firstChildElement("bottom");
             r = domElement.firstChildElement("red").text().toInt();
             g = domElement.firstChildElement("green").text().toInt();
             b = domElement.firstChildElement("blue").text().toInt();
             m_grad2ColorPicker->setColor(QColor(r, g, b));
            domElement = backgroundElement.firstChildElement("title-color");
             r = domElement.firstChildElement("red").text().toInt();
             g = domElement.firstChildElement("green").text().toInt();
             b = domElement.firstChildElement("blue").text().toInt();
             m_titleColorPicker->setColor(QColor(r, g, b));
            domElement = backgroundElement.firstChildElement("foreground-color");
             r = domElement.firstChildElement("red").text().toInt();
             g = domElement.firstChildElement("green").text().toInt();
             b = domElement.firstChildElement("blue").text().toInt();
             m_foreColorPicker->setColor(QColor(r, g, b));

            // ratio mode
            domElement = backgroundElement.firstChildElement("back-properties");
            if (domElement.isElement())
                m_backContentRatio = (Qt::AspectRatioMode)domElement.attribute("ratio").toInt();
        }
    }

    // CONTENT
    {
        // find the 'content' element
        QDomElement contentElement = canvasElement.firstChildElement("content");

        // create all content
        for (QDomElement ce = contentElement.firstChildElement(); !ce.isNull(); ce = ce.nextSiblingElement()) {

            // create the right kind of content
            AbstractContent * content = 0;
            if (ce.tagName() == "picture")
                content = createPicture(QPoint());
            else if (ce.tagName() == "text")
                content = createText(QPoint());
            else if (ce.tagName() == "webcam")
                content = createWebcam(ce.attribute("input").toInt(), QPoint());
            if (!content) {
                qWarning("Canvas::fromXml: unknown content type '%s'", qPrintable(ce.tagName()));
                continue;
            }

            // load item properties, and delete it if something goes wrong
            if (!content->fromXml(ce)) {
                m_content.removeAll(content);
                delete content;
                continue;
            }

            // restore the background element of the canvas
            if (ce.firstChildElement("set-as-background").isElement()) {
                if (m_backContent)
                    qWarning("Canvas::fromXml: only 1 element with <set-as-background/> allowed");
                else
                    setBackContent(content);
            }
        }
    }

    // refresh all
    update();
    resize(sceneSize());
    emit refreshCanvas();
}

void Canvas::renderVisible(QPainter * painter, const QRectF & target, const QRectF & source, Qt::AspectRatioMode aspectRatioMode, bool hideTools)
{
    if (hideTools) {
        clearSelection();
        setSearchPicturesVisible(false);
        foreach(QGraphicsItem *item, m_markerItems)
            item->hide();
        foreach(AbstractConfig *conf, m_configs)
            conf->hide();
    }

    RenderOpts::HQRendering = true;
    QGraphicsScene::render(painter, target, source, aspectRatioMode);
    RenderOpts::HQRendering = false;

    if (hideTools) {
        foreach(AbstractConfig *conf, m_configs)
            conf->show();
        foreach(QGraphicsItem *item, m_markerItems)
            item->show();
    }
}

QImage Canvas::renderedImage(const QSize & iSize, Qt::AspectRatioMode aspectRatioMode, bool hideTools)
{
    QImage result(iSize, QImage::Format_ARGB32);
    result.fill(0);

    QPainter painter(&result);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

    QSize targetSize = sceneSize();
    targetSize.scale(iSize, aspectRatioMode);
    int offsetX = (iSize.width() - targetSize.width()) / 2;
    int offsetY = (iSize.height() - targetSize.height()) / 2;

    QRect targetRect = QRect(offsetX, offsetY, targetSize.width(), targetSize.height());
    renderVisible(&painter, targetRect, sceneRect(), Qt::IgnoreAspectRatio, hideTools);
    painter.end();

    return result;
}

bool Canvas::printAsImage(int printerDpi, const QSize & pixelSize, bool landscape, Qt::AspectRatioMode aspectRatioMode)
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
void Canvas::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
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

        // match local and remote urls against all supported extensions
        foreach (const QUrl & url, event->mimeData()->urls()) {
            if (url.scheme() == "http" || url.scheme() == "ftp" || !url.toLocalFile().isEmpty()) {
                QString urlString = url.toString();
                foreach (const QString & extension, extensions) {
                    if (urlString.endsWith(extension, Qt::CaseInsensitive)) {
                        event->accept();
                        return;
                    }
                }
            }
        }
    }

    // check content drop
    if (event->mimeData()->hasFormat("picturesearch/idx")) {
        event->accept();
        return;
    }
}

void Canvas::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children
    event->ignore();
    QGraphicsScene::dragMoveEvent(event);

    // or continue accepting event for this
    if (!event->isAccepted()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void Canvas::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // handle by children
    event->ignore();
    QGraphicsScene::dropEvent(event);
    if (event->isAccepted())
        return;

    // handle as an own file drop event
    if (event->mimeData()->hasUrls()) {
        event->accept();
        QPoint pos = event->scenePos().toPoint();
        foreach (const QUrl & url, event->mimeData()->urls()) {
            // handle network images
            if (url.scheme() == "http" || url.scheme() == "ftp") {
                PictureContent * p = createPicture(pos);
                if (!p->loadFromNetwork(url.toString(), 0)) {
                    m_content.removeAll(p);
                    delete p;
                } else
                    pos += QPoint(30, 30);
            }

            // handle local files
            if (QFile::exists(url.toLocalFile())) {
                PictureContent * p = createPicture(pos);
                if (!p->loadPhoto(url.toLocalFile(), true, true)) {
                    m_content.removeAll(p);
                    delete p;
                } else
                    pos += QPoint(30, 30);
            }
        }
        return;
    }

    // handle as an own content drop event
    if (event->mimeData()->hasFormat("picturesearch/idx") && m_pictureSearch) {

        // get the picture service
        AbstractPictureService * pictureService = m_pictureSearch->pictureService();
        if (!pictureService)
            return;

        // download each picture
        QPoint insertPos = event->scenePos().toPoint();
        QStringList sIndexes = QString(event->mimeData()->data("picturesearch/idx")).split(",");
        foreach (const QString & sIndex, sIndexes) {
            int index = sIndex.toUInt();

            // get picture description
            QString url;
            QString title;
            int width = 0;
            int height = 0;
            if (!pictureService->imageInfo(index, &url, &title, &width, &height))
                continue;

            // get the download
            QNetworkReply * reply = pictureService->download(index);
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

void Canvas::keyPressEvent(QKeyEvent * keyEvent)
{
    QGraphicsScene::keyPressEvent(keyEvent);
    if (!keyEvent->isAccepted() && keyEvent->key() == Qt::Key_Delete)
        slotDeleteContent();
}

void Canvas::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    // first dispatch doubleclick to items
    mouseEvent->ignore();
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    // unset the background picture, if present
    setBackMode(m_backGradientEnabled ? 2 : 1);
}

void Canvas::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
    // context menu on empty area
    //if (items(event->scenePos()).isEmpty()) {
    //}
    QGraphicsScene::contextMenuEvent(event);
}


/// Scene Background & Foreground
void Canvas::drawBackground(QPainter * painter, const QRectF & exposedRect)
{
    // clip exposedRect to the scene
    QRect targetRect = sceneRect().toAlignedRect().intersect(exposedRect.toAlignedRect());

    // draw content if set
    if (m_backContent) {
        // regenerate cache if needed
        if (m_backCache.isNull() || m_backCache.size() != sceneSize())
            m_backCache = m_backContent->toPixmap(sceneSize(), m_backContentRatio);

        // paint cached background
        if (m_backContent->contentOpaque())
            painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawPixmap(targetRect, m_backCache, targetRect);
        if (m_backContent->contentOpaque())
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        return;
    }

    // draw background gradient, if enabled
    if (m_backGradientEnabled) {
        QLinearGradient lg(0, 0, 0, sceneHeight());
        lg.setColorAt(0.0, m_grad1ColorPicker->color());
        lg.setColorAt(1.0, m_grad2ColorPicker->color());
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(targetRect, lg);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        return;
    }

    // draw checkboard to simulate a transparent background
    if (!RenderOpts::ARGBWindow && !RenderOpts::HQRendering)
        painter->drawTiledPixmap(targetRect, m_backTile, QPointF(targetRect.left() % 100, targetRect.top() % 100));
}

void Canvas::drawForeground(QPainter * painter, const QRectF & exposedRect)
{
    // clip exposedRect to the scene
    QRect targetRect = sceneRect().toAlignedRect().intersect(exposedRect.toAlignedRect());

    // draw header and footer 50px bars
    if (m_topBarEnabled || m_bottomBarEnabled) {
        QColor hColor = m_foreColorPicker->color();
        hColor.setAlpha(128);
        if (m_topBarEnabled && targetRect.top() < 50)
            painter->fillRect(targetRect.left(), 0, targetRect.width(), 50, hColor);
        if (m_bottomBarEnabled && targetRect.bottom() >= sceneHeight() - 50)
            painter->fillRect(targetRect.left(), sceneHeight() - 50, targetRect.width(), 50, hColor);
    }

    // draw Title text
    if (!m_titleText.isEmpty()) {
        painter->setFont(QFont("Courier 10 Pitch", 28));
        QLinearGradient lg(0,15,0,35);
        QColor titleColor = m_titleColorPicker->color();
        lg.setColorAt(0.0, titleColor);
        lg.setColorAt(0.49, titleColor.lighter(150));
        lg.setColorAt(0.51, titleColor.darker(150));
        lg.setColorAt(1.0, titleColor);
        painter->setPen(QPen(lg, 0));
        painter->drawText(QRect(0, 0, sceneWidth(), 50), Qt::AlignCenter, m_titleText);
    }
}

void Canvas::initContent(AbstractContent * content, const QPoint & pos)
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

void Canvas::setBackContent(AbstractContent * content)
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

    // hide gradient controls
    m_grad1ColorPicker->hide();
    m_grad2ColorPicker->hide();

    // update GUI
    m_backCache = QPixmap();
    update();
    emit backModeChanged();
}

CanvasViewContent * Canvas::createCanvasView(const QPoint & pos)
{
    CanvasViewContent * d = new CanvasViewContent(this);
    initContent(d, pos);
    return d;
}

PictureContent * Canvas::createPicture(const QPoint & pos)
{
    PictureContent * p = new PictureContent(this);
    initContent(p, pos);
    connect(p, SIGNAL(flipHorizontally()), this, SLOT(slotFlipHorizontally()));
    connect(p, SIGNAL(flipVertically()), this, SLOT(slotFlipVertically()));
    connect(p, SIGNAL(requestCrop()), this, SLOT(slotCrop()));
    return p;
}

TextContent * Canvas::createText(const QPoint & pos)
{
    TextContent * t = new TextContent(this);
    initContent(t, pos);
    return t;
}

WebcamContent * Canvas::createWebcam(int input, const QPoint & pos)
{
    WebcamContent * w = new WebcamContent(input, this);
    initContent(w, pos);
    return w;
}

WordCloudContent * Canvas::createWordCloud(const QPoint & pos)
{
    WordCloudContent * w = new WordCloudContent(this);
    initContent(w, pos);
    return w;
}

void Canvas::deleteContent(AbstractContent * content)
{
    if (content) {
        // unset background if deleting its content
        if (m_backContent == content)
            setBackContent(0);

        // remove related property if deleting its content
        foreach (AbstractConfig * config, m_configs) {
            if (config->content() == content) {
                deleteConfig(config);
                break;
            }
        }

        // unlink content from lists, myself(the Scene) and memory
        m_content.removeAll(content);
        content->dispose();
    }
}

void Canvas::deleteConfig(AbstractConfig * config)
{
    if (config) {
        m_configs.removeAll(config);
        config->dispose();
    }
}

/// Slots
void Canvas::slotSelectionChanged()
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
    QList<AbstractContent *> selectedContent = projectList<QGraphicsItem, AbstractContent>(selection);
    if (!selectedContent.isEmpty()) {
        SelectionProperties * pWidget = new SelectionProperties(selectedContent);
        connect(pWidget, SIGNAL(deleteSelection()), this, SLOT(slotDeleteContent()));
        emit showPropertiesWidget(pWidget);
        return;
    }

    // or don't show anything
    emit showPropertiesWidget(0);
}

void Canvas::slotConfigureContent(const QPoint & scenePoint)
{
    // get the content and ensure it hasn't already a property window
    AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
    foreach (AbstractConfig * config, m_configs) {
        if (config->content() == content)
            return;
        // force only 1 property instance
        deleteConfig(config);
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
    connect(p, SIGNAL(requestClose()), this, SLOT(slotDeleteConfig()));
    connect(p, SIGNAL(applyLook(quint32,bool,bool)), this, SLOT(slotApplyLook(quint32,bool,bool)));
    p->show();
    p->setPos(scenePoint - QPoint(10, 10));
    p->keepInBoundaries(sceneRect().toRect());
    p->setFocus();
}

void Canvas::slotBackgroundContent()
{
    setBackContent(dynamic_cast<AbstractContent *>(sender()));
}

void Canvas::slotStackContent(int op)
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

void Canvas::slotDeleteContent()
{
    QList<AbstractContent *> selectedContent = projectList<QGraphicsItem, AbstractContent>(selectedItems());
    AbstractContent * senderContent = dynamic_cast<AbstractContent *>(sender());
    if (senderContent && !selectedContent.contains(senderContent)) {
        selectedContent.clear();
        selectedContent.append(senderContent);
    }
    if (selectedContent.size() > 1)
        if (QMessageBox::question(0, tr("Delete content"), tr("All the %1 selected content will be deleted, do you want to continue ?").arg(selectedContent.size()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;

    foreach (AbstractContent * content, selectedContent)
        deleteContent(content);
}

void Canvas::slotDeleteConfig()
{
    deleteConfig(dynamic_cast<AbstractConfig *>(sender()));
}

void Canvas::slotApplyLook(quint32 frameClass, bool mirrored, bool all)
{
    foreach (AbstractContent * content, m_content) {
        if (all || content->isSelected()) {
            if (content->frameClass() != frameClass)
                content->setFrame(FrameFactory::createFrame(frameClass));
            content->setMirrored(mirrored);
        }
    }
}

void Canvas::slotApplyEffect(const PictureEffect & effect, bool all)
{
    QList<PictureContent *> pictures = projectList<AbstractContent, PictureContent>(m_content);
    foreach (PictureContent * picture, pictures)
        if (all || picture->isSelected())
            picture->addEffect(effect);
}

void Canvas::slotCrop()
{
    QList<PictureContent *> pictures = projectList<QGraphicsItem, PictureContent>(selectedItems());
    foreach (PictureContent * picture, pictures)
        picture->crop();
}

void Canvas::slotFlipHorizontally()
{
    QList<PictureContent *> pictures = projectList<QGraphicsItem, PictureContent>(selectedItems());
    foreach (PictureContent * picture, pictures)
        picture->addEffect(PictureEffect::FlipH);
}

void Canvas::slotFlipVertically()
{
    QList<PictureContent *> pictures = projectList<QGraphicsItem, PictureContent>(selectedItems());
    foreach (PictureContent * picture, pictures)
        picture->addEffect(PictureEffect::FlipV);
}

void Canvas::slotTitleColorChanged()
{
    update(0, 0, sceneWidth(), 50);
}

void Canvas::slotForeColorChanged()
{
    update(0, 0, sceneWidth(), 50);
    update(0, sceneHeight() - 50, sceneWidth(), 50);
}

void Canvas::slotGradColorChanged()
{
    update();
}

void Canvas::slotBackContentChanged()
{
    m_backCache = QPixmap();
    update();
}

void Canvas::slotCloseIntroduction()
{
    m_helpItem->dispose();
    m_helpItem = 0;
    foreach (HighlightItem * highlight, m_highlightItems)
        highlight->deleteAfterAnimation();
    m_highlightItems.clear();
}

void Canvas::slotApplyForce()
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
