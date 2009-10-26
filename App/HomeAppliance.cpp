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
#include "Shared/RenderOpts.h"
#include "App.h"
#include "Settings.h"
#include "UrlHistoryBox.h"
#include "Workflow.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPixmap>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#define ANIMATE_PARAM(object, propName, duration, endValue) \
    {QPropertyAnimation * ani = new QPropertyAnimation(object, propName, object); \
    ani->setEasingCurve(QEasingCurve::OutBack); \
    ani->setDuration(duration); \
    ani->setEndValue(endValue); \
    ani->start(QPropertyAnimation::DeleteWhenStopped);}
#else
#define ANIMATE_PARAM(instance, propName, duration, endValue) \
    instance->setProperty(propName, endValue);
#endif


/** Home Appliance **/

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


/** Home Label **/

class HomeLabel : public AbstractContent
{
    public:
        HomeLabel(const QString & title, const QPixmap & pixmap, QGraphicsScene * scene)
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
            setAcceptsHoverEvents(true);
            resizeContents(QRect(-pixW/2, -pixH/2, pixW, pixH), false);
        }

        QString contentName() const
        {
            return "HomeLabel";
        }

        void hoverEnterEvent(QGraphicsSceneHoverEvent *)
        {
            ANIMATE_PARAM(this, "scale", 300, 1.2);
        }

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
        {
            ANIMATE_PARAM(this, "scale", 300, 1.0);
        }

        void mousePressEvent(QGraphicsSceneMouseEvent * event)
        {
            // use an already existing signal.. FIXME!
            if (event->button() == Qt::LeftButton)
                emit requestEditing();
        }

        void drawContent(QPainter * painter, const QRect & targetRect)
        {
            qreal scale = property("scale").toDouble();
            painter->setRenderHint(QPainter::SmoothPixmapTransform, scale == 1.2);
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            painter->drawPixmap(targetRect, m_pixmap);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        }

    private:
        QPixmap m_pixmap;
};


/** Home Scene **/

HomeScene::HomeScene(QObject * parent)
  : AbstractScene(parent)
{
    HomeLabel * newWordcloud = new HomeLabel(tr("New Wordcloud"), QPixmap(":/data/home-newwordcloud.png"), this);
     connect(newWordcloud, SIGNAL(requestEditing()), this, SLOT(slotNewWordcloud()));
     m_labels.append(newWordcloud);
    HomeLabel * newCanvas = new HomeLabel(tr("New Fotowall"), QPixmap(":/data/home-newcanvas.png"), this);
     connect(newCanvas, SIGNAL(requestEditing()), this, SLOT(slotNewCanvas()));
     m_labels.append(newCanvas);
    HomeLabel * wizard = new HomeLabel(tr("Wizard"), QPixmap(":/data/home-wizard.png"), this);
     connect(wizard, SIGNAL(requestEditing()), this, SLOT(slotWizard()));
     m_labels.append(wizard);
}

HomeScene::~HomeScene()
{
    qDeleteAll(m_labels);
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
    // resize but ensure a minimum size
    AbstractScene::resize(size.expandedTo(QSize(600, 200)));

    // grid placement
    const int margin = 20;
    const int count = m_labels.size();
    const int width = sceneWidth() - margin * 2;
    const int height = sceneHeight();
    const int rows = (int)sqrt((qreal)count);
    const int cols = (int)ceil((qreal)count / (qreal)rows);
    int rIdx = 0, cIdx = 0;
    for (int i = 0; i < count; i++) {
        double xPos = margin + (int)(((qreal)cIdx + 0.5) * (qreal)width / (qreal)cols);
        double yPos = (int)(((qreal)rIdx + 0.5) * (qreal)height / (qreal)rows);
        if (cols == 3 && cIdx == 1)
            yPos -= 10;
        m_labels[i]->setPos(QPointF(xPos, yPos) - m_labels[i]->boundingRect().center());
        if (++cIdx >= cols) {
            cIdx = 0;
            ++rIdx;
        }
    }
}

void HomeScene::slotNewCanvas()
{
    App::workflow->newCanvas();
}

void HomeScene::slotNewWordcloud()
{
    //App::workflow->loadCanvas(url.toString());
}

void HomeScene::slotWizard()
{
    //App::workflow->loadCanvas(url.toString());
}
