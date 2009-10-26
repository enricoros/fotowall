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
#include "Frames/StandardFrame.h"
#include "Shared/AbstractScene.h"
#include "Shared/RenderOpts.h"
#include "App.h"
#include "Settings.h"
#include "UrlHistoryBox.h"
#include "Workflow.h"

#include <QPainter>
#include <QPixmap>

//BEGIN Home Scene
class HomeContent : public AbstractContent
{
    public:
        HomeContent(const QString & title, const QPixmap & pixmap, QGraphicsScene * scene)
          : AbstractContent(scene, 0, true)
          , m_pixmap(pixmap)
        {
            // create the standard frame and set title
            setFrame(new StandardFrame2);
            setFrameTextEnabled(true);
            setFrameText(title);

            // change appearance for the home screen
            int pixW = pixmap.width();
            int pixH = pixmap.height();
            setFlags(ItemClipsChildrenToShape);
            setAcceptsHoverEvents(false);
            resizeContents(QRect(-pixW/2, -pixH/2, pixW, pixH), false);
            //rotate(-5);
        }

        QString contentName() const
        {
            return QString();
        }

        void drawContent(QPainter * painter, const QRect & targetRect)
        {
            //painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter->drawPixmap(targetRect, m_pixmap);
            //painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
        }

    private:
        QPixmap m_pixmap;
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
        QList<QGraphicsItem *> m_buttons;
};

HomeScene::HomeScene(QObject *parent)
  : AbstractScene(parent)
{
    // create buttons
    QFont font;
    font.setPointSize(font.pointSize() + 10);
    m_buttons.append(new HomeContent(tr("New Fotowall"), QPixmap(":/data/home-newcanvas.png"), this));
    m_buttons.append(new HomeContent(tr("New Wordcloud"), QPixmap(":/data/home-newwordcloud.png"), this));
    m_buttons.append(new HomeContent(tr("Wizard"), QPixmap(":/data/home-wizard.png"), this));
}

void HomeScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // draw a gray shade as background
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    QLinearGradient lg(0, 0, sceneWidth() / 2, sceneHeight());
    lg.setColorAt(0.0, QColor(192, 192, 192, RenderOpts::ARGBWindow ? 128 : 255));
    lg.setColorAt(1.0, QColor(128, 128, 128, RenderOpts::ARGBWindow ? 128 : 255));
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#if 0
    QRadialGradient rg(sceneWidth() / 2, 0, 200, sceneWidth() / 2, -100);
    rg.setColorAt(0.0, Qt::white);
    rg.setColorAt(1.0, Qt::transparent);
    painter->fillRect(rect, rg);
#endif
}

void HomeScene::resize(const QSize & size)
{
    AbstractScene::resize(size.expandedTo(QSize(300, 200)));
}

void HomeScene::resizeEvent()
{
#if 1
    // grid placement
    const int count = m_buttons.size();
    const int width = sceneWidth();
    const int height = sceneHeight();
    const int rows = (int)sqrt((qreal)count);
    const int cols = (int)ceil((qreal)count / (qreal)rows);
    int rIdx = 0, cIdx = 0;
    for (int i = 0; i < count; i++) {
        double xPos = (int)(((qreal)cIdx + 0.5) * (qreal)width / (qreal)cols);
        double yPos = (int)(((qreal)rIdx + 0.5) * (qreal)height / (qreal)rows);
        m_buttons[i]->setPos(QPointF(xPos, yPos) - m_buttons[i]->boundingRect().center());
        if (++cIdx >= cols) {
            cIdx = 0;
            ++rIdx;
        }
    }
#endif
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
