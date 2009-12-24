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

#ifndef __AbstractScene_h__
#define __AbstractScene_h__

#include <QGraphicsScene>
#include <QRectF>
#include <QSize>

class AbstractScene : public QGraphicsScene
{
    Q_OBJECT
    public:
        AbstractScene(QObject * parent = 0);

        // tells the views to update their layouts
        void adjustSceneSize();

        // scene size
        void resizeAutoFit();
        virtual void resize(const QSize & size);
        virtual void resizeEvent();
        inline int sceneWidth() const { return m_size.width(); }
        inline int sceneHeight() const { return m_size.height(); }
        inline QSize sceneSize() const { return m_size; }
        inline QRectF sceneRect() const { return m_rect; }
        inline QPointF sceneCenter() const { return m_rect.center(); }

        // how to view the scene (defaults to true)
        virtual bool sceneSelectable() const;

    Q_SIGNALS:
        void sceneSizeChanged();

    protected:
        QSize m_size;
        QRectF m_rect;
};

#endif
