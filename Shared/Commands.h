/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by TANGUY Arnaud <arn.tanguy@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __Command__
#define __Command__

#include "Shared/AbstractCommand.h"
#include "Shared/GroupedCommands.h"
#include "Shared/CommandStack.h"
#include "Shared/PictureEffect.h"
#include "Shared/ColorPickerItem.h"

#include "Canvas/Canvas.h"
#include "Canvas/AbstractContent.h"
#include "Canvas/PictureContent.h"
#include "Canvas/TextContent.h"
#include "Canvas/WebcamContent.h"
#include "Frames/FrameFactory.h"
#include <QStringList>
#include <QUrl>
#include <QDebug>

//DONE
class EffectCommand : public AbstractCommand {
    private:
        PictureContent *m_content;
        QList<PictureEffect> m_previousEffects;
        PictureEffect m_newEffect;
        QRect m_previousSize;
    public:
        EffectCommand(PictureContent *content, const PictureEffect &effect) : m_content(content)
                                                                            , m_newEffect(effect)
        {
            m_oldContent = content;
            m_previousEffects = content->effects();
            m_previousSize = content->contentRect();
        }
        void exec() {
            m_content->addEffect(m_newEffect);
        }
        void unexec() {
            // Reset the correct size (because of effects like crop).
            m_content->resizeContents(m_previousSize, false);

            m_content->addEffect(PictureEffect::ClearEffects);
            foreach(PictureEffect effect, m_previousEffects) {
                m_content->addEffect(effect);
            }
        }
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_content == oldContent) {
                m_content = dynamic_cast<PictureContent *> (newContent);
            }
        }
        QString name() const {
            return tr("New effects");
        }
};

//DONE
/* This commands manage the text of the TextContent */
class TextCommand : public AbstractCommand {
    private:
        /* Private vars */
        TextContent *m_content;
        QString m_previousText, m_newText;
    public:
        TextCommand(TextContent *content, QString newText) : m_content(content), m_newText (newText)
        {
            m_oldContent = m_content;
            m_previousText = content->toHtml();
        }
        void exec() {
            m_content->setHtml(m_newText);
        }
        void unexec() {
            m_content->setHtml(m_previousText);
        }
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_content == oldContent) {
                m_content = dynamic_cast<TextContent *>(newContent);
            }
        }
        QString name() const {
            return tr("Text changed");
        }
};

//DONE
/* This commands manage transformations: rotations and scaling */
class TransformCommand : public AbstractCommand {
    private:
        /* Private vars */
        QTransform m_previous, m_new;
    public:
        TransformCommand(AbstractContent *content, QTransform& previous_matrix, QTransform& new_matrix) : m_previous(previous_matrix)
                                                              , m_new(new_matrix)
        { m_content = content; }
        void exec() {
            m_content->setTransform(m_new);
        }
        void unexec() {
            m_content->setTransform(m_previous);
        }
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_content == oldContent) {
                m_content = newContent;
            }
        }
        QString name() const {
            return tr("Transformation");
        }
};

//DONE
class RotateAndResizeCommand : public AbstractCommand {
    private:
        /* Private vars */
        const qreal m_pAngle, m_nAngle;
        const QRect m_pRect, m_nRect;
    public:
        RotateAndResizeCommand(AbstractContent *content,
                                const qreal pAngle, const qreal nAngle,
                                const QRect pRect, const QRect nRect) : m_pAngle(pAngle) , m_nAngle(nAngle)
                                                          , m_pRect(pRect), m_nRect(nRect)
       { m_content = content; }
        void exec() {
            m_content->setRotation(m_nAngle);
            m_content->resizeContents(m_nRect);
        }
        void unexec() {
            m_content->setRotation(m_pAngle);
            m_content->resizeContents(m_pRect);
        }
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_content == oldContent) {
                m_content = newContent;
            }
        }
        QString name() const {
            return tr("Rotation and Resize");
        }
};

//DONE
/* This command manges movements of the content */
class MotionCommand : public AbstractCommand {
    private:
        /* Private vars */
        AbstractContent *m_content;
        QPointF m_previous, m_newMotion;
    public:
        MotionCommand(AbstractContent *content, QPointF previous, QPointF newMotion) : m_content(content)
                                                                                   ,m_previous(previous)
                                                                                   ,m_newMotion(newMotion)
        {}
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_content == oldContent) {
                qDebug() << "\tMotionCommand: Replacing content... Oh Yeah!";
                m_content = newContent;
            }
        }
        void exec() {
            m_content->setPos(m_newMotion);
        }
        void unexec() {
            m_content->setPos(m_previous);
        }
        QString name() const {
            return tr("Move");
        }
        QString description() const {
            QString previousStr = QString::number(m_previous.x())+QString::number(m_previous.y());
            QString newMotionStr = QString::number(m_newMotion.x())+QString::number(m_newMotion.y());
            return tr("Moved from %1 to %2").arg(previousStr, newMotionStr);
        }
};

//DONE
class DeleteContentCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        QDomElement m_contentElt;
    public:
        DeleteContentCommand(AbstractContent *content, Canvas *canvas) : m_canvas(canvas)
        {m_oldContent = content; m_content = content; }

        void exec() {
            qDebug() << "Saving and deleting image";
            // Saves the adress of the current content to be able to replace it by the new one in the stack later.
            m_oldContent = m_content;

            // Get the content xml info, in order to recreate it correctly
            QDir t = QDir::currentPath();
            QDomDocument doc;
            m_contentElt = doc.createElement("content");
            m_content->toXml(m_contentElt, t); // XXX: why does it need a path ???

            m_canvas->deleteContent(m_content);
            m_content=0;
        }

        void unexec() {
            if(m_content==0) {
                qDebug() << "Restoring image";
                qDebug() << "DeleteContent: Content is deleted, recreate it";
                m_content = m_canvas->addContentFromXml(m_contentElt);
                qDebug() << "DeleteContent: Content restored";
                if(m_oldContent != m_content) {
                    qDebug() << "The new content is at a different adress than the previous one, update pointers in stack";
                    CommandStack::instance().replaceContent(m_oldContent, m_content);
              }
            }
        }
        QString name() const {
            return tr("Delete content");
        }
};

//DONE
/* This command manages creation (and hidding when unexec) of new image content */
class NewImageCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        const QStringList m_imagesPath;
        GroupedCommands *m_gc;
        bool m_created;
    public:
        NewImageCommand(Canvas *canvas, const QStringList& paths) : m_canvas(canvas), m_imagesPath(paths)
                                                                  , m_created(false)
        { m_gc = new GroupedCommands(); }
        void exec() {
            if(!m_created) {
                QList<PictureContent *> images = m_canvas->addPictureContent(m_imagesPath);
                foreach (PictureContent *content, images) {
                    DeleteContentCommand *c = new DeleteContentCommand(content, m_canvas);
                    m_gc->addCommand(c);
                }
                m_created = true;
            } else {
                m_gc->unexec();
            }
        }
        void unexec() {
            m_gc->exec();
        }
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
        }
        QString name() const {
            return tr("Add images");
        }
        QString description() const {
            QString desc;
            foreach (QString path, m_imagesPath) {
               desc += path += "\n";
            }
            return desc;
        }
};

/*class NewNetworkImageCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        const QList<QUrl> m_imagesPath;
        QList<AbstractContent *> m_images;
        QPoint m_pos;
    public:
        NewNetworkImageCommand(Canvas *desk, const QList<QUrl>& paths, QPoint pos) : m_canvas(desk) , m_imagesPath(paths)
                                                                                    , m_pos(pos)
        {}
        void exec() {
            if(m_images.isEmpty()) {
                m_images = m_canvas->addNetworkPictures(m_imagesPath, m_pos);
            } else {
                foreach(AbstractContent *image, m_images) {
                    image->show();
                }
            }
        }
        void unexec() {
            // Instead of deleting images, keep them in memory.
            // It's faster to restaure, and fix some other problems if redo is used :
            // the stack would contains commands using the old deleted item...
            foreach(AbstractContent *image, m_images) {
                image->hide();
            }
            //m_canvas->removeContents(m_images);
        }
        QString name() const {
            return tr("Add network images");
        }
        QString description() const {
            QString desc;
            foreach(QUrl url, m_imagesPath) {
                desc += url.toString() += "\n";
            }
            return desc;
        }
};

[> Manage the image created by flickr. Do not create it, Canvas::dropEvent does that <]
class NewFlickrImageCommand : public AbstractCommand {
    private:
        PictureContent * m_image;
    public:
        NewFlickrImageCommand(PictureContent * image) : m_image(image)
        {}
        void exec() {
            m_image->show();
        }
        void unexec() {
            // Instead of deleting images, keep them in memory.
            // It's faster to restaure, and fix some other problems if redo is used :
            // the stack would contains commands using the old deleted item...
            m_image->hide();
        }
        QString name() const {
            return tr("Add Flickr image '%1'").arg(m_image->frameText());
        }
};*/

/* This command manages creation (and hidding when unexec) of new text content */
//DONE
class NewTextCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        TextContent *m_content;
        QDomElement m_contentElt;
        bool m_created;
    public:
        NewTextCommand(Canvas *canvas) : m_canvas(canvas), m_content(0), m_created(false)
        { m_oldContent = m_content; }
        void exec() {
            if(m_content==0) {
                if(!m_created) {
                    m_content = m_canvas->addTextContent();
                    m_created=true;
                } else {
                    qDebug() << "DeleteContent: Content is deleted, recreate it";
//XXX: secure dynamic_cast
                    m_content = dynamic_cast<TextContent *>(m_canvas->addContentFromXml(m_contentElt));
                    qDebug() << "DeleteContent: Content restored";
                    if(m_oldContent != m_content) {
                        qDebug() << "The new content is at a different adress than the previous one, update pointers in stack";
                        CommandStack::instance().replaceContent(m_oldContent, m_content);
                    }
                }
            }
        }
        void unexec() {
            // Saves the adress of the current content to be able to replace it by the new one in the stack later.
            m_oldContent = m_content;

            // Get the content xml info, in order to recreate it correctly
            QDir t = QDir::currentPath();
            QDomDocument doc;
            m_contentElt = doc.createElement("content");
            m_content->toXml(m_contentElt, t); // XXX: why does it need a path ???
            m_canvas->deleteContent(m_content);
            m_content=0;
        }
        AbstractContent *content() {
            return m_content;
        }
        QString name() const {
            return tr("Add text");
        }
};

//XXX: todo (but I have to get a webcam working first
/* This command manages creation (and hidding when unexec) of new webcam content */
class NewWebcamCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        int m_webcamIndex;
        WebcamContent *m_content;
    public:
        NewWebcamCommand(Canvas *canvas, int webcamIndex) : m_canvas(canvas), m_webcamIndex(webcamIndex)
                                                            , m_content(0)
        {}

        bool setContent(AbstractContent *content) {
            WebcamContent *c = dynamic_cast<WebcamContent *>(content);
            if(c) {
                m_content = c;
                return true;
            } else
                return false;
        }
        AbstractContent *content() const {
            return m_content;
        }

        void exec() {
            if(m_content == 0)
                m_content = m_canvas->addWebcamContent(m_webcamIndex);
            else
                m_content->show();
        }
        void unexec() {
            m_content->hide();
            //AbstractContent *c = dynamic_cast<AbstractContent *>(m_content);
            //m_canvas->removeContent(c);
        }

        QString name() const {
            return tr("Add webcam");
        }
};


//DONE
/* This command manges movements of the content */
class FrameCommand : public AbstractCommand {
    private:
        /* Private vars */
        quint32 m_previousClass, m_newClass;
        bool m_previousMirror, m_newMirror;
    public:
        FrameCommand(AbstractContent *content, quint32 newClass, bool mirrored) : m_newClass(newClass)
                                                                                , m_newMirror(mirrored)
        {
            m_content = content;
            m_previousClass = content->frameClass();
            m_previousMirror = content->mirrored();
        }
        void exec() {
            if (!m_newClass)
                return;
            m_content->setFrame(FrameFactory::createFrame(m_newClass));
            m_content->setMirrored(m_newMirror);
        }
        void unexec() {
            if (!m_previousClass)
                return;
            m_content->setFrame(FrameFactory::createFrame(m_previousClass));
            m_content->setMirrored(m_previousMirror);
        }

        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_content == oldContent) {
                m_content = newContent;
            }
        }

        QString name() const {
            return tr("Change frame/mirror");
        }
};

//DONE
/* Manage background content */
class BackgroundContentCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        AbstractContent *m_previousContent, *m_newContent;
    public:
        BackgroundContentCommand(Canvas *desk, AbstractContent *previousContent, AbstractContent *newContent)
            : m_canvas(desk)
            , m_previousContent(previousContent)
            , m_newContent(newContent)
        {}
        void exec() {
            m_canvas->setBackContent(m_newContent);
        }
        void unexec() {
            m_canvas->setBackContent(m_previousContent);
        }
        void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
            if(m_previousContent == oldContent) {
                m_previousContent = newContent;
            } else if(m_newContent == oldContent) {
                m_newContent = newContent;
            }
        }
        QString name() const {
            return tr("Change background");
        }

};

//DONE
class BackgroundRatioCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        const Qt::AspectRatioMode m_previousRatio, m_newRatio;
    public:
        BackgroundRatioCommand(Canvas *desk, const Qt::AspectRatioMode& previousRatio, const Qt::AspectRatioMode& newRatio)
            : m_canvas(desk)
            , m_previousRatio(previousRatio)
            , m_newRatio(newRatio)
        {}
        void exec() {
            m_canvas->setBackContentRatio(m_newRatio);
        }
        void unexec() {
            m_canvas->setBackContentRatio(m_previousRatio);
        }
        QString name() const {
            return tr("New background ratio");
        }
};

//DONE
class BackgroundModeCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        const Canvas::BackMode m_previousMode, m_newMode;
    public:
        BackgroundModeCommand(Canvas *desk, const Canvas::BackMode& previousMode, const Canvas::BackMode& newMode)
            : m_canvas(desk)
            , m_previousMode(previousMode)
            , m_newMode(newMode)
        {}
        void exec() {
            m_canvas->setBackMode(m_newMode);
        }
        void unexec() {
            m_canvas->setBackMode(m_previousMode);
        }
        QString name() const {
            return tr("New background mode");
        }
};

//DONE
class DecoTopBarCommand : public AbstractCommand {
    Canvas *m_canvas;
    bool m_state;

    public:
    DecoTopBarCommand(Canvas *canvas, bool state) : m_canvas(canvas)
                                                  , m_state(state)
    {}
    void exec() {
        m_canvas->setTopBarEnabled(m_state);
    }
    void unexec() {
        m_canvas->setTopBarEnabled(!m_state);
    }
    QString name() const {
        return tr("Top bar enabled/disabled");
    }
};
//DONE
class DecoBottomBarCommand : public AbstractCommand {
    Canvas *m_canvas;
    bool m_state;

    public:
    DecoBottomBarCommand(Canvas *canvas, bool state) : m_canvas(canvas)
                                                  , m_state(state)
    {}
    void exec() {
        m_canvas->setBottomBarEnabled(m_state);
    }
    void unexec() {
        m_canvas->setBottomBarEnabled(!m_state);
    }
    QString name() const {
        return tr("Bottom bar enabled/disabled");
    }
};

//DONE
class DecoTitleCommand : public AbstractCommand {
    private:
        /* Private vars */
        Canvas *m_canvas;
        QString m_previousText, m_newText;
    public:
        DecoTitleCommand(Canvas *canvas, QString newText) : m_canvas(canvas), m_newText (newText)
        {
            m_previousText = m_canvas->titleText();
        }
        void exec() {
            m_canvas->setTitleText(m_newText);
        }
        void unexec() {
            m_canvas->setTitleText(m_previousText);
        }
        QString name() const {
            return tr("Text changed");
        }
};

//DONE
class ColorPickerCommand : public AbstractCommand {
    ColorPickerItem *m_cItem;
    const QColor m_pColor, m_nColor;

    public:
    ColorPickerCommand( ColorPickerItem *item, const QColor pCol, const QColor nCol)
        : m_cItem(item), m_pColor(pCol), m_nColor(nCol)
    {}

    void exec() {
        m_cItem->setColor(m_nColor);
    }
    void unexec() {
        m_cItem->setColor(m_pColor);
    }
    QString name() const {
        return tr("Color changed");
    }
};

//DONE
class StackCommand : public AbstractCommand {
    int m_pZ, m_nZ;
    public:
    StackCommand(AbstractContent *c, int pZ, int nZ) : m_pZ(pZ), m_nZ(nZ)
    { m_content = c; }

    void exec() {
        m_content->setZValue(m_nZ);
    }
    void unexec() {
        m_content->setZValue(m_pZ);
    }
    QString name() const {
        return tr("Stack order");
    }
    void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
        if(m_content == oldContent) {
            m_content = newContent;
        }
    }
};

//XXX: Delete modifications to apply
class ShapeCommand : public AbstractCommand {
    TextContent* m_content;
    QList<QPointF >  m_pCps, m_nCps;
    public:
    ShapeCommand(TextContent *c, const QList<QPointF >& pCps, const QList<QPointF>& nCps) : m_content(c)
                                                       , m_pCps(pCps), m_nCps(nCps)
    { }

    void exec() {
        m_content->setControlPoints(m_nCps);
    }
    void unexec() {
        m_content->setControlPoints(m_pCps);
    }
};

#endif

