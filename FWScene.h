/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FWScene_h__
#define __FWScene_h__

#include <QGraphicsScene>
#include <QDataStream>
#include <QPainter>
#include <QRect>
class FWFoto;
class ColorPickerItem;

class FWScene : public QGraphicsScene
{
    Q_OBJECT
    public:
        FWScene(QObject * parent = 0);
        ~FWScene();

        // resize the scene to 0,0,size
        void resize(const QSize & size);

        // set the title text
        QString titleText() const;
        void setTitleText(const QString & text);

        // save and restore
        void save(QDataStream & data) const;
        void restore(QDataStream & data);

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent * event );
        void dragMoveEvent( QGraphicsSceneDragDropEvent * event );
        void dropEvent( QGraphicsSceneDragDropEvent * event );
        void drawBackground( QPainter * painter, const QRectF & rect );
        void drawForeground( QPainter * painter, const QRectF & rect );

    private:
        QList<FWFoto *> m_photos;
        ColorPickerItem * m_titleColorPicker;
        ColorPickerItem * m_foreColorPicker;
        ColorPickerItem * m_grad1ColorPicker;
        ColorPickerItem * m_grad2ColorPicker;
        QSize m_size;
        QRectF m_rect;
        QString m_titleText;

    private Q_SLOTS:
        void slotDeleteFoto();
        void slotTitleColorChanged();
        void slotForeColorChanged();
        void slotGradColorChanged();
};

#endif
