/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *  Modified by Tanguy Arnaud <phparnsk8@gmail.com>, see CHANGLOG to have  *
 *  summary of the modification.										   *	
 *  																	   *	
 *  This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FWFoto_h__
#define __FWFoto_h__

#include <QGraphicsItem>
#include <QBrush>
#include <QPointF>
#include <QObject>
#include "define.h"

#include "FWPictureInfo.h"
#include "CPixmap.h"

class FWButton;
class QGraphicsTextItem;

// resizable "photo" item
class FWFoto : public QObject, public QGraphicsItem
{
	Q_OBJECT
	public:
		FWFoto( QGraphicsItem * parent = 0 );
		~FWFoto();

		// photo
		void loadPhoto(const QString & fileName, bool keepRatio = true, bool setName= true);

		// save/restore
		void save(QDataStream & data) const;
		void restore(QDataStream & data);

		void setImgNum(int num);
		void setBackgroundPhoto(QString);

		// ::QGraphicsItem
		QRectF boundingRect() const;
		void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
		void wheelEvent( QGraphicsSceneWheelEvent * event );
		void keyPressEvent( QKeyEvent * event );
		void keyReleaseEvent(QKeyEvent *event);
		void mousePressEvent(QGraphicsSceneMouseEvent*);

Q_SIGNALS:
		void deletePressed();
		void imageSelected(int);

	private:
		void relayoutContents();
		QString     m_fileName;
		QString m_tmpSaveName;
		CPixmap *   m_photo;
		QPixmap *m_backgroundPhoto;
		//IplImage *m_CVImg;
		QPixmap     m_cachedPhoto;
		FWPictureInfo m_pictureInfo;

		QSize       m_size;
		FWButton *  m_scaleButton;
		FWButton *  m_rotateButton;
		QGraphicsTextItem * m_textItem;
		//QTimer *    m_scaleRefreshTimer;
		bool        m_scaling;
		bool 		m_CtrlKey;
		int m_imgNum;  //It's the position of picture in the FWScene::m_photos. It's used to send signals, to know what image is selected for the butons like menu...


	private Q_SLOTS:
		void slotResize(const QPointF & controlPoint);
		void slotResizeProportional(const QPointF & controlPoint);
		void slotRotate(const QPointF & controlPoint);
		void slotResetSize();
		void slotResetRotation();
		void slotResizeEnded();
		void slotSaveImage();


	public slots:
		void slotToNVG(); //convert the picture in level of grey
		void slotToBlackAndWhite();
		void slotLuminosity(int value);
		void slotInvertColors(); 
		void slotFlipH();
		void slotFlipV();
		void slotRotate(int angle=90);

};





class FWButton : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        FWButton( FWFoto * parent, const QBrush & brush );
	FWButton(FWFoto *parent, const QString &iconPath); 
	~FWButton();

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
        void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
        void mousePressEvent( QGraphicsSceneMouseEvent * event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event );

    Q_SIGNALS:
        void dragging(const QPointF & point);
        void reset();

    private:
        FWFoto *    m_parent;
        QBrush      m_brush;
        QPointF     m_startPos;
	QPixmap *m_iconPixmap;
};




#endif
