/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Modified by Tanguy Arnaud <phparnsk8@gmail.com>, see CHANGLOG to have 
 *   summary of the modification.					
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FWScene_h__
#define __FWScene_h__

#include <QFileDialog>
#include <QGraphicsScene>
#include <QDataStream>
#include <QPainter>
#include <QRect>
#include <QTimer>
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

	void FWupdate() {
		update();
	}
        // set the title text
        QString titleText() const;
        void setTitleText(const QString & text);

        // save and restore
	void loadPictures(QStringList *fileNames);  //To load pictures with the button instead of drag and drop
        void save(QDataStream & data) const;
        void restore(QDataStream & data);

	void setCurentImageAsBackground();

protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent * event );
        void dragMoveEvent( QGraphicsSceneDragDropEvent * event );
        void dropEvent( QGraphicsSceneDragDropEvent * event );
        void drawBackground( QPainter * painter, const QRectF & rect );
	void drawBackgroundImage(QPainter *painter);
        void drawForeground( QPainter * painter, const QRectF & rect );

private:
        QList<FWFoto *> m_photos;
	int m_imgSelectedNumber;
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
	void slotImageSelected(int);

	void slotCallChangeImageBackground(QString);
public:
	void callSlotToNVG();
	void callSlotToBlackAndWhite();
	void callSlotLuminosity();
	void callSlotInvertColors();
	void callSlotFlipH();
	void callSlotFlipV();
	void callSlotRotate(int angle=90);
};

#endif
