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
#include "Shared/ButtonsDialog.h"

#include <QCommonStyle>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QStyleOption>

/// The style used by the SceneView's rubberband selection
class RubberBandStyle : public QCommonStyle
{
    public:
        void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const
        {
            if (element != CE_RubberBand)
                return QCommonStyle::drawControl(element, option, painter, widget);
            painter->save();
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
{
    // customize widget
    setInteractive(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::SmoothPixmapTransform */);
    setDragMode(QGraphicsView::RubberBandDrag);
    setAcceptDrops(true);
    setFrameStyle(QFrame::NoFrame);

    // don't autofill the view with the Base brush
    QPalette pal;
    pal.setBrush(QPalette::Base, Qt::NoBrush);
    setPalette(pal);

    // use own style for drawing the RubberBand (opened on the viewport)
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
    m_abstractScene = scene;
    QGraphicsView::setScene(m_abstractScene);
    adjustSceneSize();
}

AbstractScene * SceneView::scene() const
{
    return m_abstractScene;
}

AbstractScene * SceneView::takeScene()
{
    AbstractScene * m_backScene = m_abstractScene;
    setScene(0);
    return m_backScene;
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

    // change viewport widget and mode
    QWidget * viewport = m_openGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget();
    viewport->setStyle(m_style);
    setViewport(viewport);
    setViewportUpdateMode(m_openGL ? QGraphicsView::FullViewportUpdate : QGraphicsView::MinimalViewportUpdate);
    update();
}
#else
void SceneView::setOpenGL(bool) {};
#endif

void SceneView::sceneConstraintsUpdated()
{
    // TODO
    qWarning("TODO SceneView::sceneConstraintsUpdated");
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

void SceneView::resizeEvent(QResizeEvent * event)
{
    adjustSceneSize();
    QGraphicsView::resizeEvent(event);
}

void SceneView::adjustSceneSize()
{
    if (!m_abstractScene)
        return;

    QSize viewportSize = viewport()->contentsRect().size();

    // do a real calculation
    QSize sceneSize = viewportSize; //QSize(qMax(viewportSize.width(), 600), qMax(viewportSize.height(), 400));

    // change the scrollbars policy
    Qt::ScrollBarPolicy sPolicy = ((sceneSize.width() > viewportSize.width()) ||
                                  (sceneSize.height() > viewportSize.height())) ?
                                  Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff;
    setVerticalScrollBarPolicy(sPolicy);
    setHorizontalScrollBarPolicy(sPolicy);

    // change size
    m_abstractScene->resize(sceneSize);
}
