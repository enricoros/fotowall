/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "HomeAppliance.h"

#include "Canvas/PictureContent.h"
#include "Shared/AbstractScene.h"
#include "Shared/RenderOpts.h"
#include "App.h"
#include "Settings.h"
#include "UrlHistoryBox.h"
#include "Workflow.h"

#include <QGraphicsGridLayout>
#include <QPainter>
#include <QSvgRenderer>

//BEGIN Home Scene
class HomeContent : public AbstractContent
{
    public:
        HomeContent(const QString & svgFileName, QGraphicsScene * scene)
          : AbstractContent(scene, 0, true)
          , m_renderer(new QSvgRenderer)
        {
            m_renderer->load(svgFileName);
        }

        // ::AbstractContent
        QString contentName() const { return QString(); }
        void drawContent(QPainter * painter, const QRect & targetRect)
        {
            //painter->fillRect(targetRect.adjusted(10, 10, -10, -10), Qt::blue);
            //m_renderer->render(painter);
        }

    private:
        QSvgRenderer * m_renderer;

};

class HomeScene : public AbstractScene
{
    public:
        HomeScene(QObject * parent = 0);

        // ::QGraphicsScene
        void drawBackground(QPainter *painter, const QRectF &rect);

        // ::AbstractScene
        void resize(const QSize & size);
        void resizeEvent();

    private:
        QGraphicsGridLayout * m_lay;
        QList<HomeContent *> m_content;
};

HomeScene::HomeScene(QObject *parent)
  : AbstractScene(parent)
  , m_lay(new QGraphicsGridLayout)
{
    // button: new Canvas
    HomeContent * c = new HomeContent(":/data/home-newcanvas.svg", this);
    m_content.append(c);
    m_content.append(new HomeContent(":/data/home-newcanvas.svg", this));
    m_content.append(new HomeContent(":/data/home-newcanvas.svg", this));
}

void HomeScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    QLinearGradient lg(0, 0, 0, sceneHeight());
    lg.setColorAt(0.0, QColor(192, 192, 192, RenderOpts::ARGBWindow ? 128 : 255));
    lg.setColorAt(1.0, QColor(128, 128, 128, RenderOpts::ARGBWindow ? 128 : 255));
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void HomeScene::resize(const QSize & size)
{
    AbstractScene::resize(size.expandedTo(QSize(300, 200)));
}

void HomeScene::resizeEvent()
{
    const int count = m_content.size();
    const int width = sceneWidth();
    const int height = sceneHeight();
    const int rows = (int)sqrt((qreal)count);
    const int cols = (int)ceil((qreal)count / (qreal)rows);
    int rIdx = 0, cIdx = 0;
    for (int i = 0; i < count; i++) {
        double xPos = (int)(((qreal)cIdx + 0.5) * (qreal)width / (qreal)cols);
        double yPos = (int)(((qreal)rIdx + 0.5) * (qreal)height / (qreal)rows);
        m_content[i]->setPos(xPos, yPos);
        if (++cIdx >= cols) {
            cIdx = 0;
            ++rIdx;
        }
    }
}
//END Home Scene




HomeAppliance::HomeAppliance(QObject *parent)
    : PlugGui::AbstractAppliance(parent)
    , m_scene(new HomeScene)
    , m_historyBox(0)
{
    // configure the appliance
    sceneSet(m_scene);

    // create the History Box, if enough history
    QList<QUrl> recentUrls = App::settings->recentFotowallUrls();
    if (!recentUrls.isEmpty()) {
        m_historyBox = new UrlHistoryBox(recentUrls);
        m_historyBox->setTitle(tr("RECENT FILES"));
        m_historyBox->setBorderFlags(0x0000);
        m_historyBox->setCheckable(false);
        QPalette pal;
        pal.setBrush(QPalette::Window, QColor(0, 0, 0, 16));
        m_historyBox->setPalette(pal);
        m_historyBox->setAutoFillBackground(true);
        connect(m_historyBox, SIGNAL(urlClicked(QUrl)), this, SLOT(slotLoadUrl(const QUrl &)));
        topbarAddWidget(m_historyBox);
    }
}

HomeAppliance::~HomeAppliance()
{
    delete m_historyBox;
    delete m_scene;
}

void HomeAppliance::slotLoadUrl(const QUrl & url)
{
    // handle non-fotowall files
    if (!App::isFotowallFile(url.toString())) {
        qWarning("HomeAppliance::slotLoadUrl: don't know how to load URL '%s'", qPrintable(url.toString()));
        return;
    }

    // load Fotowall file
    App::workflow->loadCanvas(url.toString());
}
