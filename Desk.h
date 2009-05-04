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
#include <QTime>
class AbstractContent;
class AbstractProperties;
struct CEffect;
class ColorPickerItem;
class HelpItem;
class HighlightItem;
class PictureContent;
class QTimer;
class TextContent;
class VideoContent;
class WebContentSelectorItem;

class Desk : public QGraphicsScene
{
    Q_OBJECT
    public:
        friend class XmlRead;
        friend class XmlSave;
        Desk(QObject * parent = 0);
        ~Desk();

        // add content
        void addPictures(const QStringList & fileNames);
        void addTextContent();
        void addVideoContent(int input);

        // selectors
        void setWebContentSelectorVisible(bool visible);
        bool webContentSelectorVisible() const;

        // resize the scene to 0,0,size
        void resize(const QSize & size);

        // item interaction
        void selectAllContent(bool selected = true);

        // arrangement
        void setForceFieldEnabled(bool enabled);
        bool forceFieldEnabled() const;

        // decorations
        void setBackGradientEnabled(bool enabled);
        bool backGradientEnabled() const;
        void setTopBarEnabled(bool enabled);
        bool topBarEnabled() const;
        void setBottomBarEnabled(bool enabled);
        bool bottomBarEnabled() const;
        void setTitleText(const QString & text);
        QString titleText() const;

        // save, restore, load, help
        void showIntroduction();
        void blinkBackGradients();

        // get and set the project mode (CD cover, DVD,...).
        enum Mode { ModeNormal = 0, ModeCD = 1, ModeDVD = 2, ModeExactSize = 3 };
        Mode projectMode() const;
        void setProjectMode(Mode mode);

        // render the Desk, but not the invisible items
        void renderVisible(QPainter * painter, const QRectF & target = QRectF(), const QRectF & source = QRectF(), Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio);
        QImage renderedImage(const QSize & size, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio);
        bool printAsImage(int printerDpi, const QSize & pixelSize, bool landscape, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio);

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent * event );
        void dragMoveEvent( QGraphicsSceneDragDropEvent * event );
        void dropEvent( QGraphicsSceneDragDropEvent * event );
        void keyPressEvent( QKeyEvent * keyEvent );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent * event );
        void drawBackground( QPainter * painter, const QRectF & rect );
        void drawForeground( QPainter * painter, const QRectF & rect );

    private:
        void initContent(AbstractContent * content, const QPoint & pos);
        PictureContent * createPicture(const QPoint & pos);
        TextContent * createText(const QPoint & pos);
        VideoContent * createVideo(int input, const QPoint & pos);
        void setDVDMarkers();
        void clearMarkers();
        QList<AbstractContent *> m_content;
        QList<AbstractProperties *> m_properties;
        QList<HighlightItem *> m_highlightItems;
        HelpItem * m_helpItem;
        AbstractContent * m_backContent;
        ColorPickerItem * m_titleColorPicker;
        ColorPickerItem * m_foreColorPicker;
        ColorPickerItem * m_grad1ColorPicker;
        ColorPickerItem * m_grad2ColorPicker;
        bool m_topBarEnabled;
        bool m_bottomBarEnabled;
        bool m_backGradientEnabled;
        QSize m_size;
        QRectF m_rect;
        QString m_titleText;
        QPixmap m_backCache;
        Mode m_projectMode;
        QList<QGraphicsItem *> m_markerItems;   // used by some modes to show information items, which won't be rendered
        WebContentSelectorItem * m_webContentSelector;
        QTimer * m_forceFieldTimer;
        QTime m_forceFieldTime;

    private Q_SLOTS:
        void slotConfigureContent(const QPoint & scenePoint);
        void slotBackgroundContent();
        void slotStackContent(int);
        void slotDeleteContent();
        void slotDeleteProperties();
        void slotApplyLook(quint32 frameClass, bool mirrored, bool allContent);
        void slotApplyEffect(const CEffect & effect, bool allPictures);
        void slotFlipHorizontally();
        void slotFlipVertically();

        void slotTitleColorChanged();
        void slotForeColorChanged();
        void slotGradColorChanged();

        void slotCloseIntroduction();
        void slotApplyForce();
};

#endif
