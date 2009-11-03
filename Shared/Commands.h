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
#include "Shared/PictureEffect.h"

#include "Canvas/Canvas.h"
#include "Canvas/AbstractContent.h"
#include "Canvas/PictureContent.h"
#include "Canvas/TextContent.h"
#include "Canvas/WebcamContent.h"
#include "Frames/FrameFactory.h"
#include <QStringList>
#include <QUrl>

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
        QString name() {
            return tr("New effects");
        }
};

/* This commands manage the text of the TextContent */
class TextCommand : public AbstractCommand {
    private:
        /* Private vars */
        TextContent *m_content;
        QString m_previousText, m_newText;
    public:
        TextCommand(TextContent *content, QString newText) : m_content(content), m_newText (newText)
        {
            m_previousText = content->toHtml();
        }
        void exec() {
            m_content->setHtml(m_newText);
        }
        void unexec() {
            m_content->setHtml(m_previousText);
        }
        QString name() {
            return tr("Text changed");
        }
};

/* This commands manage transformations: rotations and scaling */
class TransformCommand : public AbstractCommand {
    private:
        /* Private vars */
        AbstractContent *m_content;
        QTransform m_previous, m_new;
    public:
        TransformCommand(AbstractContent *content, QTransform& previous_matrix, QTransform& new_matrix) : m_content(content)
                                                              , m_previous(previous_matrix)
                                                              , m_new(new_matrix)
        {}
        void exec() {
            m_content->setTransform(m_new);
        }
        void unexec() {
            m_content->setTransform(m_previous);
        }
        QString name() {
            return tr("Transformation");
        }
};

class RotateAndResizeCommand : public AbstractCommand {
    private:
        /* Private vars */
        AbstractContent *m_content;
        const qreal m_pAngle, m_nAngle;
        const QRect m_pRect, m_nRect;
    public:
        RotateAndResizeCommand(AbstractContent *content,
                                const qreal pAngle, const qreal nAngle,
                                const QRect pRect, const QRect nRect) : m_content(content)
                                                          , m_pAngle(pAngle) , m_nAngle(nAngle)
                                                          , m_pRect(pRect), m_nRect(nRect)
       {}
        void exec() {
            m_content->setRotation(m_nAngle);
            m_content->resizeContents(m_nRect);
        }
        void unexec() {
            m_content->setRotation(m_pAngle);
            m_content->resizeContents(m_pRect);
        }
        QString name() {
            return tr("Rotation and Resize");
        }
};

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
        void exec() {
            m_content->setPos(m_newMotion);
        }
        void unexec() {
            m_content->setPos(m_previous);
        }
        QString name() {
            return tr("Move");
        }
        QString description() {
            QString previousStr = QString::number(m_previous.x())+QString::number(m_previous.y());
            QString newMotionStr = QString::number(m_newMotion.x())+QString::number(m_newMotion.y());
            return tr("Moved from %1 to %2").arg(previousStr, newMotionStr);
        }
};

/* This command manages creation (and hidding when undo) of new image content */
class NewImageCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        const QStringList m_imagesPath;
        QList<PictureContent *> m_images;
    public:
        NewImageCommand(Canvas *desk, const QStringList& paths) : m_canvas(desk), m_imagesPath(paths)
        {}
        void exec() {
            if(m_images.isEmpty()) {
                m_images = m_canvas->addPictureContent(m_imagesPath);
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
        QString name() {
            return tr("Add images");
        }
        QString description() {
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
        QString name() {
            return tr("Add network images");
        }
        QString description() {
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
        QString name() {
            return tr("Add Flickr image '%1'").arg(m_image->frameText());
        }
};*/

/* This command manages creation (and hidding when undo) of new text content */
class NewTextCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        TextContent *m_content;
    public:
        NewTextCommand(Canvas *desk) : m_canvas(desk), m_content(0)
        {}
        void exec() {
            if(m_content == 0)
                m_content = m_canvas->addTextContent();
            else
                m_content->show();
        }
        void unexec() {
            //AbstractContent *c = dynamic_cast<AbstractContent *>(m_content);
            //m_canvas->removeContent(c);
            m_content->hide();
        }
        QString name() {
            return tr("Add text");
        }
};

/* This command manages creation (and hidding when undo) of new webcam content */
class NewWebcamCommand : public AbstractCommand {
    private:
        Canvas *m_canvas;
        WebcamContent *m_content;
    public:
        NewWebcamCommand(Canvas *desk) : m_canvas(desk), m_content(0)
        {}
        void exec() {
            if(m_content == 0)
                m_content = m_canvas->addWebcamContent(0);
            else
                m_content->show();
        }
        void unexec() {
            m_content->hide();
            //AbstractContent *c = dynamic_cast<AbstractContent *>(m_content);
            //m_canvas->removeContent(c);
        }

        QString name() {
            return tr("Add webcam");
        }
};

/* Hide the content instead of deleting */
class DeleteContentCommand : public AbstractCommand {
    private:
        AbstractContent *m_content;
    public:
        DeleteContentCommand(AbstractContent *content) : m_content(content)
        {}
        void exec() {
            m_content->hide();
        }
        void unexec() {
            m_content->show();
        }
        QString name() {
            return tr("Delete content");
        }
};

/* This command manges movements of the content */
class FrameCommand : public AbstractCommand {
    private:
        /* Private vars */
        AbstractContent *m_content;
        quint32 m_previousClass, m_newClass;
        bool m_previousMirror, m_newMirror;
    public:
        FrameCommand(AbstractContent *content, quint32 newClass, bool mirrored) : m_content(content)
                                                                            , m_newClass(newClass)
                                                                            , m_newMirror(mirrored)

        {
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
        QString name() {
            return tr("Change frame");
        }
};

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
        QString name() {
            return tr("Change background");
        }

};

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
        QString name() {
            return tr("New background ratio");
        }
};

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
        QString name() {
            return tr("New background mode");
        }
};

#endif
