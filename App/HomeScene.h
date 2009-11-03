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

#ifndef __HomeScene_h__
#define __HomeScene_h__

#include "Shared/AbstractScene.h"
#include <QList>
#include <QPixmap>
#include <QRect>

/**
    \brief The Scene showing the {New Canvas, New Wordcloud, etc..} items.
*/
class HomeScene : public AbstractScene
{
    Q_OBJECT
    public:
        HomeScene(QObject * parent = 0);
        ~HomeScene();

        // ::QGraphicsScene
        void drawBackground(QPainter *painter, const QRectF &rect);
        void drawForeground(QPainter *painter, const QRectF &rect);
        void keyPressEvent(QKeyEvent *event);

        // ::AbstractScene
        void resize(const QSize & size);
        bool sceneSelectable() const;

    Q_SIGNALS:
        void keyPressed(int qtKey);
        void startCanvas();
        void startWordcloud();
        void startWizard();

    private:
        QList<QGraphicsItem *> m_labels;
        QPixmap m_logoPixmap;
        QRect m_logoRect;
};

#endif
