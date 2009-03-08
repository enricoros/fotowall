/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *  Modified by Tanguy Arnaud <phparnsk8@gmail.com>, see CHANGLOG to have  *
 *  summary of the modification.										   * 
 *  This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FWFoto.h"
#include <QPainter>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QUrl>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QKeyEvent>
#include <math.h>
//#include "opencvtoqt.h"
// from FotoWall.cpp
extern bool globalExportingFlag;
extern bool drawPictureBorder;

#define SCALE_X     (- 29 - 5)
#define SCALE_Y     (- 8 -5)
#define ROTATE_X    (- 8 - 5)
#define ROTATE_Y    (- 8 -5)
#define FW_MARGIN  	10 
#define FW_LABH     30



class MyTextItem : public QGraphicsTextItem {
    public:
        MyTextItem(QGraphicsItem * parent = 0)
            : QGraphicsTextItem(parent)
        {
        }

        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
            painter->save();
            painter->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform );
            
            QGraphicsTextItem::paint(painter, option, widget);
            painter->restore();
        }
};

FWFoto::FWFoto(QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_photo(0)
    , m_size(200, 150)
    //, m_scaleRefreshTimer(0)
    , m_scaling(false)
{
	m_CtrlKey = false;
	//m_tmpSaveName = TMPDIR + "tmpFotowallImg.png";
	m_backgroundPhoto = new QPixmap;
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);


	// create child items
	m_scaleButton = new FWButton(this, "./data/maximize.png");
	connect(m_scaleButton, SIGNAL(dragging(const QPointF&)), this, SLOT(slotResize(const QPointF&)));
	connect(m_scaleButton, SIGNAL(reset()), this, SLOT(slotResetSize()));
	m_rotateButton = new FWButton(this, "./data/rotate_left.png");
	connect(m_rotateButton, SIGNAL(dragging(const QPointF&)), this, SLOT(slotRotate(const QPointF&)));
	connect(m_rotateButton, SIGNAL(reset()), this, SLOT(slotResetRotation()));
	m_textItem = new MyTextItem(this);
	m_textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
	QFont f("Serif");
	f.setPointSizeF(7.5);
	m_textItem->setFont(f);
	m_textItem->setDefaultTextColor(QColor(0,0,0));
	m_textItem->setPlainText(tr("          "));

	relayoutContents();
}
FWFoto::~FWFoto() {
}


void FWFoto::loadPhoto(const QString & fileName, bool keepRatio, bool ) {
	delete m_photo;
	m_photo = new CPixmap(fileName);
	if (m_photo->isNull()) {
		QMessageBox::warning(0, tr("Error"), tr("The file has not been loaded. The format may be not supported"));
		delete this;
	}
	m_fileName = fileName;
	if (keepRatio)
		slotResetSize();
	m_cachedPhoto = 0;
	update();

}
void FWFoto::save(QDataStream & data) const {
    data << m_size;
    data << pos();
    data << transform();
    data << zValue();
    data << m_fileName;
    data << m_textItem->toPlainText();
	//added
	data<<drawPictureBorder;
//	data << m_pictureInfo.isNVG();
//		data << m_pictureInfo.isInverted();
}       	
void FWFoto::restore(QDataStream & data) {
    prepareGeometryChange();
    data >> m_size;
    relayoutContents();
    QPointF p;
    data >> p;
    setPos(p);
    QTransform t;
    data >> t;
    setTransform(t);
    qreal zVal;
    data >> zVal;
    setZValue(zVal);
    QString fileName;
    data >> fileName;
    loadPhoto(fileName);
    QString text;
    data >> text;
	data >> drawPictureBorder;

//	bool isNVG;
//	data>>isNVG;
//	if(isNVG) 
//		slotToNVG();
//
//	bool isInverted=false;
//	data>>isInverted;
//	if(isInverted)
//		slotInvertColors();
   m_textItem->setPlainText(text);
    update();

}
void FWFoto::setImgNum(int num) {
	m_imgNum = num;
}
void FWFoto::setBackgroundPhoto(QString path) {
	delete m_backgroundPhoto;
	m_backgroundPhoto = new QPixmap(path);
	if (m_backgroundPhoto) {
		if(m_backgroundPhoto->width() > 4*m_size.width() || m_backgroundPhoto->height() > 4*m_size.height()) {
			int answer = QMessageBox::question(0, tr("Image really big"), tr("The selected image has a %1 x %2  resolution.\nAre you sure you want to set this image as background?").arg(m_backgroundPhoto->width()).arg(m_backgroundPhoto->height()), QMessageBox::Yes | QMessageBox::No);
			if(answer==QMessageBox::No) {
				delete m_backgroundPhoto;
				m_backgroundPhoto = new QPixmap;
			}
		}
	}
	m_cachedPhoto=0;
	update();
}
QRectF FWFoto::boundingRect() const {
	if(m_backgroundPhoto->isNull())
		return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
	else {
		if(m_size.width()<m_backgroundPhoto->width() && m_size.height() < m_backgroundPhoto->height())
			return QRectF(-m_backgroundPhoto->width()/2, -m_backgroundPhoto->height()/2,m_backgroundPhoto->width(), m_backgroundPhoto->height()); 
		else
			return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());

	}
}
void FWFoto::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/) {
	if (drawPictureBorder) {
		if(m_backgroundPhoto->isNull()) {
			//painter->fillRect(boundingRect(), QColor(0,0,0,64));
			QLinearGradient lg(0, -m_size.height() / 2, 0, m_size.height() / 2);
			lg.setColorAt(0.0, QColor(128,128,128, 200));
			lg.setColorAt(1.0, QColor(255,255,255, 200));

			painter->fillRect(boundingRect(), lg);
			//  painter->fillRect(boundingRect().adjusted(5, 5, -5, -5), lg);
		}
		else {
			const QPoint point(-m_backgroundPhoto->width()/2,-m_backgroundPhoto->height()/2);
			const QPixmap backgroundImage(*m_backgroundPhoto);
			painter->drawPixmap(point,backgroundImage);
		}
	}
    if (!m_photo)
        return;
    // draw high-resolution photo when exporting png
    QRect targetRect;
	   targetRect.setRect(-m_size.width()/2 + FW_MARGIN, -m_size.height()/2 + FW_MARGIN, m_size.width() - 2*FW_MARGIN, m_size.height() - 2*FW_MARGIN - FW_LABH);
	
    if (globalExportingFlag){
        painter->drawPixmap(targetRect, *m_photo);
        return;
    }

    // draw photo using caching and deferred rescales
    if (m_scaling) {
        if (!m_cachedPhoto.isNull()) 
            painter->drawPixmap(targetRect, m_cachedPhoto);
    } else {
        if (m_cachedPhoto.isNull() || m_cachedPhoto.size() != targetRect.size())
            m_cachedPhoto = m_photo->scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect.topLeft(), m_cachedPhoto);
    }
}
void FWFoto::wheelEvent(QGraphicsSceneWheelEvent * event){
    int newZValue = (int)(zValue() - event->delta() / 120);
    setZValue(newZValue);
}
void FWFoto::keyReleaseEvent(QKeyEvent *event){
	if (event->key() == Qt::Key_Control) 
		m_CtrlKey = false;
}
void FWFoto::mousePressEvent(QGraphicsSceneMouseEvent *) {
	emit imageSelected(m_imgNum);
}
void FWFoto::keyPressEvent(QKeyEvent * event) {
	if (event->key() == Qt::Key_B)
		slotToNVG();
	if(event->key() == Qt::Key_W)
		slotToBlackAndWhite();
	if (event->key() == Qt::Key_I) 
		slotInvertColors();
	if (event->key() == Qt::Key_H)
		slotFlipH();
	if (event->key() == Qt::Key_V)
		slotFlipV();
	if (event->key() == Qt::Key_R)
		slotRotate(90);
	if (event->key() == Qt::Key_L)
		slotRotate(-90);
	if (event->key() == Qt::Key_S)
		slotSaveImage();
	if (event->key() == Qt::Key_Delete)
		emit deletePressed();
	if (event->key() == Qt::Key_Control) 
		m_CtrlKey = true;

	event->accept();
}
void FWFoto::relayoutContents() {
    m_scaleButton->setPos(m_size.width() / 2.0 + SCALE_X, m_size.height() / 2.0 + SCALE_Y);
    m_rotateButton->setPos(m_size.width() / 2.0 + ROTATE_X, m_size.height() / 2.0 + ROTATE_Y);
    m_textItem->setPos(-m_size.width() / 2.0 + FW_MARGIN, m_size.height() / 2.0 - FW_MARGIN - FW_LABH + FW_MARGIN);
}


void FWFoto::slotResizeProportional(const QPointF & controlPoint) {
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = m_rotateButton->pos();
    if (newPos == refPos)
        return;

    // determine the new size
	int newWidth = 	(newPos.x() - SCALE_X) * 2.0;
	int newHeight = (m_size.height() * newWidth)/m_size.width();
    QSize newSize = QSize(newWidth,newHeight);
	
    if (newSize.width() < (2*FW_MARGIN + 40))
        newSize.setWidth(2*FW_MARGIN + 40);
	
		if (newSize.height() < (2*FW_MARGIN + 30 + FW_LABH))
			newSize.setHeight(2*FW_MARGIN + 30 + FW_LABH);
    // change geometry
    m_scaling = true;
    prepareGeometryChange();
    m_size = newSize;
    relayoutContents();
    update();

    // start refresh timer
//    if (!m_scaleRefreshTimer) {
//        m_scaleRefreshTimer = new QTimer(this);
//        connect(m_scaleRefreshTimer, SIGNAL(timeout()), this, SLOT(slotResizeEnded()));
//        m_scaleRefreshTimer->setSingleShot(true);
//    
//	}
//	m_scaleRefreshTimer->start(400);
}
void FWFoto::slotResize(const QPointF & controlPoint) {
	if (m_CtrlKey) {
		slotResizeProportional(controlPoint);	
	}
	else {
		QPointF newPos = mapFromScene(controlPoint);
		QPointF refPos = m_rotateButton->pos();
		if (newPos == refPos)
			return;

		// determine the new size
		QSize newSize = QSize((newPos.x() - SCALE_X) * 2.0, (newPos.y() - SCALE_Y) * 2.0);
		if (newSize.width() < (2*FW_MARGIN + 40))
			newSize.setWidth(2*FW_MARGIN + 40);
		if (newSize.height() < (2*FW_MARGIN + 30 + FW_LABH))
			newSize.setHeight(2*FW_MARGIN + 30 + FW_LABH);
		if (newSize == m_size)
			return;

		// change geometry
		m_scaling = true;
		prepareGeometryChange();
		m_size = newSize;
		relayoutContents();
		update();

		// start refresh timer
		//if (!m_scaleRefreshTimer) {
		//	m_scaleRefreshTimer = new QTimer(this);
		//	connect(m_scaleRefreshTimer, SIGNAL(timeout()), this, SLOT(slotResizeEnded()));
		//	m_scaleRefreshTimer->setSingleShot(true);
		//}
	//m_scaleRefreshTimer->start(400);
	}
}
void FWFoto::slotRotate(const QPointF & controlPoint) {
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = m_rotateButton->pos();
    if (newPos == refPos)
        return;

    // set item rotation (set rotation relative to current)
    qreal refAngle = atan2(refPos.y(), refPos.x());
    qreal newAngle = atan2(newPos.y(), newPos.x());
    rotate(180.0 * (newAngle - refAngle) / M_PI);
}
void FWFoto::slotResetSize() {
    int width = m_size.width();
    int height = m_size.height();
    if (m_photo) {
        qreal ratio = (qreal)m_photo->width() / (qreal)m_photo->height();
        if (ratio != 0.0) {
            int hfw = (int)(((qreal)width - 20.0) / ratio);
            height = FW_MARGIN + hfw + FW_LABH + FW_MARGIN;
        }
    }
    prepareGeometryChange();
    m_size = QSize(width, height);
    relayoutContents();
    update();
}
void FWFoto::slotResetRotation() {
    QTransform ident;
    setTransform(ident, false);
}
void FWFoto::slotResizeEnded() {
    m_scaling = false;
    update();
}
void FWFoto::slotSaveImage() {
		QString fileName = QFileDialog::getSaveFileName(0, tr("Select Save Path"), QDir::current().path(), tr("PNG File (*.png)"));
		if (fileName.isNull())
			return;
		if (!fileName.endsWith(".png", Qt::CaseInsensitive))
			fileName += ".png";
		if(!m_photo->save(fileName))
			QMessageBox::warning(0, tr("Save failled"), tr("An error as occured during the save"));
}

void FWFoto::slotToNVG() {
	if(m_photo->isNVG()) {
		delete m_photo;
		m_photo = new CPixmap(m_fileName);
		m_photo->setNVG(false);
	}
	else {
		m_photo->toNVG();
	}
	m_cachedPhoto = 0;
	update();
}

void FWFoto::slotToBlackAndWhite() {
	if(m_photo->isBlackAndWhite()) {
	 	delete m_photo;
	 	m_photo = new CPixmap(m_fileName);
	 	m_photo->setBlackAndWhite(false);
	}
	else {
	 	m_photo->toBlackAndWhite();
	}
	m_cachedPhoto = 0;
	update();
}

void FWFoto::slotLuminosity(int value) {
	m_photo->luminosity(value);
	m_cachedPhoto = 0;
	update();
}
void FWFoto::slotInvertColors() {
	m_photo->invertColors();
	m_cachedPhoto = 0;
	update();
	
}

void FWFoto::slotFlipH() {
	m_photo->flipH();
	m_cachedPhoto = 0;
	update();
}
void FWFoto::slotFlipV() {
	m_photo->flipV();
	m_cachedPhoto = 0;
	update();
}
void FWFoto::slotRotate(int angle) {
	// set item rotation (set rotation relative to current)
	QMatrix matrix;
	matrix.rotate(angle);
	m_photo = new CPixmap(m_photo->transformed(matrix));
	slotResetSize();
	m_cachedPhoto = 0;
	update();
}


FWButton::FWButton(FWFoto * parent, const QBrush & brush)
    : QGraphicsItem(parent)
    , m_brush(brush)
{
    setAcceptsHoverEvents(true);
    m_iconPixmap = new QPixmap;
}
FWButton::FWButton(FWFoto *parent, const QString &iconPath):QGraphicsItem(parent) 
{
	setAcceptsHoverEvents(true);
	m_iconPixmap = new QPixmap(iconPath);
}
QRectF FWButton::boundingRect() const {
	if (drawPictureBorder) 
		return QRectF(-12, -12, 16, 16);
	else
	   return QRectF(-20,-20,16,16);
}
FWButton::~FWButton() {
	delete m_iconPixmap;
}
void FWButton::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/) {
	if (option->state & QStyle::State_MouseOver) {
		if(!m_iconPixmap->isNull()) {
			
			QRectF rect = boundingRect();
			painter->drawPixmap(rect.x(), rect.y(), *m_iconPixmap);
		
			
		}
		else {
			painter->setBrush(m_brush);
			painter->setPen(Qt::white);
			painter->setOpacity(0.8);
			painter->drawEllipse(boundingRect());
			painter->setOpacity(1.0);
		}
	}

}
void FWButton::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	event->accept();
	m_startPos = event->scenePos();
}
void FWButton::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
    if (m_startPos.isNull())
        return;
    event->accept();
    emit dragging(event->scenePos());
}
void FWButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
    event->accept();
    m_startPos = QPointF();
}
void FWButton::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) {
    event->accept();
    emit reset();
}

