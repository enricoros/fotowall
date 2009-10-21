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

#ifndef __SceneView_h__
#define __SceneView_h__

#include <QGraphicsView>
#include <QTime>
class AbstractScene;
class QVBoxLayout;
class RubberBandStyle;

class SceneView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(bool openGL READ openGL WRITE setOpenGL)
    public:
        SceneView(QWidget * parent = 0);
        ~SceneView();

        // sets the scene used by this view
        void setScene(AbstractScene * scene);
        AbstractScene * scene() const;
        AbstractScene * takeScene();

        // enable/disable OpenGL acceleration
        bool supportsOpenGL() const;
        bool openGL() const;
        void setOpenGL(bool enabled);

        // layout widgets inside this
        void addOverlayWidget(QWidget * widget, bool top = true);

    Q_SIGNALS:
        void heavyRepaint();

    protected:
        // ::QGraphicsView
        void drawForeground(QPainter * painter, const QRectF & rect);
        // ::QWidget
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);

    private:
        bool m_openGL;
        AbstractScene * m_abstractScene;
        RubberBandStyle * m_style;
        QVBoxLayout * m_viewportLayout;
        QTime m_paintTime;
        QTimer * m_heavyTimer;
        int m_heavyCounter;

    private Q_SLOTS:
        // layout scene and scrollbars
        void layoutScene();
};

#endif
