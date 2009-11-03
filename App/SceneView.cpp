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

#include "SceneView.h"

#include "Shared/AbstractScene.h"

#include <QApplication>
#include <QCommonStyle>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QStyleOption>
#include <QTimer>
#include <QVBoxLayout>

/// The style used by the SceneView's rubberband selection
class RubberBandStyle : public QCommonStyle
{
    public:
        void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const
        {
            if (element != CE_RubberBand)
                return QCommonStyle::drawControl(element, option, painter, widget);
            painter->save();
            // ### Qt WORKAROUND this unbreaks the OpenGL rubberband drawing
            painter->resetTransform();
            QColor color = option->palette.color(QPalette::Highlight);
            painter->setPen(color);
            color.setAlpha(80);
            painter->setBrush(color);
            painter->drawRect(option->rect.adjusted(0,0,-1,-1));
            painter->restore();
        }
        int styleHint(StyleHint hint, const QStyleOption * option, const QWidget * widget, QStyleHintReturn * returnData) const
        {
            if (hint == SH_RubberBand_Mask)
                return false;
            return QCommonStyle::styleHint(hint, option, widget, returnData);
        }
};


SceneView::SceneView(QWidget * parent)
  : QGraphicsView(parent)
  , m_openGL(false)
  , m_abstractScene(0)
  , m_style(new RubberBandStyle)
  , m_viewportLayout(new QVBoxLayout)
  , m_heavyTimer(0)
  , m_heavyCounter(0)
{
    // customize widget
    setInteractive(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setDragMode(RubberBandDrag);
    setAcceptDrops(true);
    setFrameStyle(QFrame::NoFrame);

    // don't autofill the view with the Base brush
    QPalette pal;
    pal.setBrush(QPalette::Base, Qt::NoBrush);
    setPalette(pal);

    // use own style for drawing the RubberBand, and our layout
    m_viewportLayout->setContentsMargins(0, 4, 0, 4);
    m_viewportLayout->setSpacing(0);
    viewport()->setLayout(m_viewportLayout);
    viewport()->setStyle(m_style);

    // can't activate the cache mode by default, since it inhibits dynamical background picture changing
    //setCacheMode(CacheBackground);
}

SceneView::~SceneView()
{
    delete m_style;
}

void SceneView::setScene(AbstractScene * scene)
{
    if (m_abstractScene == scene)
        return;

    // disconnect previous scene
    if (m_abstractScene)
        disconnect(m_abstractScene, 0, this, 0);

    // use scene
    m_abstractScene = scene;
    QGraphicsView::setScene(m_abstractScene);
    if (m_abstractScene) {
        connect(m_abstractScene, SIGNAL(destroyed(QObject*)), this, SLOT(slotSceneDestroyed(QObject *)));
        connect(m_abstractScene, SIGNAL(geometryChanged()), this, SLOT(layoutScene()));
        layoutScene();
    }
}

AbstractScene * SceneView::scene() const
{
    return m_abstractScene;
}

AbstractScene * SceneView::takeScene()
{
    AbstractScene * m_scene = m_abstractScene;
    setScene(0);
    return m_scene;
}

bool SceneView::supportsOpenGL() const
{
#ifdef QT_OPENGL_LIB
    return true;
#else
    return false;
#endif
}

bool SceneView::openGL() const
{
    return m_openGL;
}

#ifdef QT_OPENGL_LIB
#include <QGLWidget>
void SceneView::setOpenGL(bool enabled)
{
    // skip if already ok
    if (m_openGL == enabled)
        return;
    m_openGL = enabled;

    // change viewport widget and transfer style and layout
    QWidget * newViewport = m_openGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget();
    newViewport->setLayout(m_viewportLayout);
    newViewport->setStyle(m_style);
    setViewport(newViewport);
    setViewportUpdateMode(m_openGL ? FullViewportUpdate : MinimalViewportUpdate);

    // transparent background for raster, standard Base on opengl
    QPalette pal = qApp->palette();
#if QT_VERSION < 0x040600
    // WORKAROUND Qt <= 4.6-beta1
    if (m_openGL)
        pal.setBrush(QPalette::Base, pal.window());
    else
#endif
        pal.setBrush(QPalette::Base, Qt::NoBrush);
    setPalette(pal);

    // issue an update, just in case..
    update();
}
#else
void SceneView::setOpenGL(bool) {};
#endif

void SceneView::addOverlayWidget(QWidget * widget, bool top)
{
    Qt::Alignment align = Qt::AlignLeft;
    if (top)
        align |= Qt::AlignTop;
    else
        align |= Qt::AlignBottom;
    m_viewportLayout->insertWidget(0, widget, 0, align);
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

void SceneView::drawForeground(QPainter * painter, const QRectF & rect)
{
    // base impl: draw Scene's foreground
    QGraphicsView::drawForeground(painter, rect);

    // the first time create the Shadow Tile
    static QPixmap shadowTile;
    if (shadowTile.isNull()) {
        shadowTile = QPixmap(64, 8);
        shadowTile.fill(Qt::transparent);
        QPainter shadowPainter(&shadowTile);
        drawVerticalShadow(&shadowPainter, 64, 8);
    }

    // find out if we have a drawing offset (we draw in Scene coords, and scene may be translated)
    int y = mapToScene(0, 0).y();

    // blend the shadow tile
    if (rect.top() < (y + 8))
        painter->drawTiledPixmap(rect.left(), y, rect.width(), 8, shadowTile);
}

void SceneView::paintEvent(QPaintEvent * event)
{
    // start the measuring time
#if 0
    const bool measureTime = event->rect().size() == viewport()->size();
#else
    const bool measureTime = true;
#endif
    if (measureTime)
        m_paintTime.start();

    // do painting
    QGraphicsView::paintEvent(event);

    // handle measurement
    if (!measureTime)
        return;
    if (m_paintTime.elapsed() < 100) {
        m_heavyCounter = 0;
        return;
    }

    // handle slow painting
    if (++m_heavyCounter > 6) {
        m_heavyCounter = -100;
        if (!m_heavyTimer) {
            m_heavyTimer = new QTimer(this);
            m_heavyTimer->setSingleShot(true);
            connect(m_heavyTimer, SIGNAL(timeout()), this, SIGNAL(heavyRepaint()));
        }
    }
    if (m_heavyTimer)
        m_heavyTimer->start(1000);
}

void SceneView::resizeEvent(QResizeEvent * event)
{
    QGraphicsView::resizeEvent(event);
    layoutScene();
}

void SceneView::layoutScene()
{
    if (!m_abstractScene)
        return;

    // change size
    QSize viewportSize = viewport()->contentsRect().size();
    m_abstractScene->resize(viewportSize);

    // change the scrollbars policy
    QSize sceneSize = m_abstractScene->sceneSize();
    bool scrollbarsNeeded = (sceneSize.width() > viewportSize.width()) || (sceneSize.height() > viewportSize.height());
    Qt::ScrollBarPolicy sPolicy = scrollbarsNeeded ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff;
    setVerticalScrollBarPolicy(sPolicy);
    setHorizontalScrollBarPolicy(sPolicy);

    // change the selection/scrolling policy
    if (m_abstractScene->sceneSelectable())
        setDragMode(RubberBandDrag);
    else
        setDragMode(scrollbarsNeeded ? ScrollHandDrag : NoDrag);

    // update screen
    update();
}

void SceneView::slotSceneDestroyed(QObject * object)
{
    // if there is a scene and it's being deleted, don't reference to it anymore
    if (m_abstractScene && static_cast<AbstractScene *>(object) == m_abstractScene)
        m_abstractScene = 0;
}
