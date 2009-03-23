/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __Desk_h__
#define __Desk_h__

#include <QGraphicsScene>
#include <QDataStream>
#include <QPainter>
#include <QPixmap>
#include <QRect>
class HelpItem;
class HighlightItem;
class PictureItem;
class PicturePropertiesItem;
class ColorPickerItem;

class Desk : public QGraphicsScene
{
    Q_OBJECT
    public:
        Desk(QObject * parent = 0);
        ~Desk();

        // resize the scene to 0,0,size
        void resize(const QSize & size);

        // set the title text
        QString titleText() const;
        void setTitleText(const QString & text);

        // save, restore, load, help
        void save(QDataStream & data) const;
        void restore(QDataStream & data);
        void loadPictures(const QStringList & fileNames);
        void showHelp();

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent * event );
        void dragMoveEvent( QGraphicsSceneDragDropEvent * event );
        void dropEvent( QGraphicsSceneDragDropEvent * event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * mouseEvent );
        void drawBackground( QPainter * painter, const QRectF & rect );
        void drawForeground( QPainter * painter, const QRectF & rect );

    private:
        PictureItem * createPicture(const QPoint & pos);
        QList<PictureItem *> m_pictures;
        QList<PicturePropertiesItem *> m_properties;
        QList<HighlightItem *> m_highlightItems;
        HelpItem * m_helpItem;
        PictureItem * m_backPicture;
        ColorPickerItem * m_titleColorPicker;
        ColorPickerItem * m_foreColorPicker;
        ColorPickerItem * m_grad1ColorPicker;
        ColorPickerItem * m_grad2ColorPicker;
        QSize m_size;
        QRectF m_rect;
        QString m_titleText;
        QPixmap m_backCache;

    private Q_SLOTS:
        void slotConfigurePicture(const QPoint & scenePoint);
        void slotBackgroundPicture();
        void slotStackPicture(int);
        void slotDeletePicture();
        void slotDeleteProperties();
        // Apply a frame to all images
        void slotApplyAll(quint32 frameClass, bool mirrored);
        // Apply an effect to all images
        void slotApplyEffectToAll(int effectClass);

        void slotTitleColorChanged();
        void slotForeColorChanged();
        void slotGradColorChanged();

        void slotCloseHelp();
};

#endif
