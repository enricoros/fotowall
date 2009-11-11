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

#include "HomeScene.h"

#include "Canvas/AbstractContent.h"
#include "Frames/StandardFrame.h"
#include "Shared/RenderOpts.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>

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
            setFrameTextReadonly(true);
            setFrameText(title);

            // incremental change over AbstractContent
            setFlag(QGraphicsItem::ItemIsMovable, false);
            setFlag(QGraphicsItem::ItemIsSelectable, false);
            setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
            const int pixW = pixmap.width();
            const int pixH = pixmap.height();
            resizeContents(QRect(-pixW/2, -pixH/2, pixW, pixH), false);
        }

        QString contentName() const
        {
            return "HomeLabel";
        }

        void hoverEnterEvent(QGraphicsSceneHoverEvent *)
        {
            ANIMATE_PARAM(this, "scale", 300, 1.2);
            //ANIMATE_PARAM(this, "rotation", 300, (-20 + (qrand() % 41)));
        }

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
        {
            ANIMATE_PARAM(this, "scale", 300, 1.0);
            //ANIMATE_PARAM(this, "rotation", 300, 0.0);
        }

        void mousePressEvent(QGraphicsSceneMouseEvent * event)
        {
            // use an already existing signal.. FIXME!
            if (event->button() == Qt::LeftButton) {
#if 0
                ANIMATE_PARAM(this, "rotation", 400, 300);
                ANIMATE_PARAM(this, "opacity", 500, 0.0);
                QTimer::singleShot(300, this, SIGNAL(requestEditing()));
#else
                requestEditing();
#endif
            }
        }

        void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode /*ratio*/)
        {
            painter->setRenderHint(QPainter::SmoothPixmapTransform, property("scale").toDouble() == 1.2);
            painter->drawPixmap(targetRect, m_pixmap);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
        }

    private:
        QPixmap m_pixmap;
};


HomeScene::HomeScene(QObject * parent)
  : AbstractScene(parent)
  , m_logoPixmap(":/data/home-logo.png")
{
    HomeLabel * newWordcloud = new HomeLabel(tr("Wordcloud"), QPixmap(":/data/home-newwordcloud.png"), this);
     connect(newWordcloud, SIGNAL(requestEditing()), this, SIGNAL(startWordcloud()));
     newWordcloud->setEnabled(false);
     newWordcloud->setOpacity(0.2);
     m_labels.append(newWordcloud);

    HomeLabel * newCanvas = new HomeLabel(tr("Create"), QPixmap(":/data/home-newcanvas.png"), this);
     connect(newCanvas, SIGNAL(requestEditing()), this, SIGNAL(startCanvas()));
     m_labels.append(newCanvas);

    HomeLabel * wizard = new HomeLabel(tr("Wizard (coming soon)"), QPixmap(":/data/home-wizard.png"), this);
     connect(wizard, SIGNAL(requestEditing()), this, SIGNAL(startWizard()));
     wizard->setEnabled(false);
     wizard->setOpacity(0.2);
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
    QLinearGradient lg(0, 0, 0, sceneHeight());
    lg.setColorAt(0.0, QColor(192, 192, 192, RenderOpts::ARGBWindow ? 200 : 255));
    lg.setColorAt(1.0, QColor(128, 128, 128, RenderOpts::ARGBWindow ? 200 : 255));
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#if 0
    QRadialGradient rg(sceneWidth() / 2, 0, 200, sceneWidth() / 2, -100);
    rg.setColorAt(0.0, Qt::white);
    rg.setColorAt(1.0, Qt::transparent);
    painter->fillRect(rect, rg);
#endif
}

void HomeScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    // draw top-center logo
    if (m_logoRect.isValid() && m_logoRect.intersects(rect.toRect()))
        painter->drawPixmap(m_logoRect.topLeft(), m_logoPixmap);
}

void HomeScene::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    emit keyPressed(event->key());
}

void HomeScene::resize(const QSize & size)
{
    // resize but ensure a minimum size
    AbstractScene::resize(size.expandedTo(QSize(600, 200)));

    // grid placement
    const int margin = 20;
    const int count = m_labels.size();
    const int width = sceneWidth() - margin * 2;
    const int height = sceneHeight() - margin;
    const int rows = (int)sqrt((qreal)count);
    const int cols = (int)ceil((qreal)count / (qreal)rows);
    int rIdx = 0, cIdx = 0;
    for (int i = 0; i < count; i++) {
        double xPos = margin + (int)(((qreal)cIdx + 0.5) * (qreal)width / (qreal)cols);
        double yPos = margin + (int)(((qreal)rIdx + 0.5) * (qreal)height / (qreal)rows);
        if (cols == 3 && cIdx != 1)
            yPos += 10;
        m_labels[i]->setPos(QPointF(xPos, yPos) - m_labels[i]->boundingRect().center());
        if (++cIdx >= cols) {
            cIdx = 0;
            ++rIdx;
        }
    }

    // logo rectangle
    if (!m_logoPixmap.isNull()) {
        int top = 0;
        if (m_labels.size() < 2)
            top = (height - rows * 200) / 10;
        else
            top = qMax((qreal)0, (m_labels[1]->sceneBoundingRect().top() - m_logoPixmap.height()) / 2);
        m_logoRect = QRect((sceneWidth() - m_logoPixmap.width()) / 2, top, m_logoPixmap.width(), m_logoPixmap.height());
    }
}

bool HomeScene::sceneSelectable() const
{
    return false;
}
