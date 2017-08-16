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
#include "Canvas/WordcloudContent.h"
#include "Frames/FrameFactory.h"
#include <QStringList>
#include <QUrl>
#include <QAction>
#include <QDebug>

/**
 * TODO
 * - Type of canvas (cd, dvd...)
 * - CanvasViewContent
 */

//DONE
class EffectCommand: public AbstractCommand {
private:
    PictureContent *m_content;
    QList<PictureEffect> m_previousEffects;
    PictureEffect m_newEffect;
    QRect m_previousSize;
public:
    EffectCommand(PictureContent *content, const PictureEffect &effect) :
            m_content(content), m_newEffect(effect) {
        m_previousEffects = content->effects();
        m_previousSize = content->contentRect();
    }
    void exec() {
        if (m_content == 0)
        {
          qDebug() << "EffectCommand::exec - null content";
            return;
        }
        m_content->addEffect(m_newEffect);
    }
    void unexec() {
      qDebug() << "unexec new effect on " << m_content;
      if (m_content == 0)
      {
          qDebug() << "EffectCommand::unexec - null content";
          return;
      }
      // Reset the correct size (because of effects like crop).
      m_content->resizeContents(m_previousSize, false);
      qDebug() << "content resized";

      m_content->addEffect(PictureEffect::ClearEffects);
      qDebug() << "effect cleared";
      foreach(PictureEffect effect, m_previousEffects) {
        qDebug() << "add effect";
        m_content->addEffect(effect);
      }
      qDebug() << "unexec new effect done";
    }
    virtual AbstractContent* content() const
    {
      return m_content;
    }
    void replaceContent(AbstractContent *oldContent,
            AbstractContent *newContent) {
        if (m_content == oldContent) {
            m_content = 0;
            m_content = dynamic_cast<PictureContent *>(newContent);
        }
    }
    QString name() const {
        return tr("New effects");
    }
};

class OpacityCommand: public AbstractCommand {
private:
    qreal previousOpacity;
    qreal newOpacity;
public:
    OpacityCommand(AbstractContent *content, qreal pOpacity, qreal nOpacity) {
       setContent(content);
       previousOpacity = pOpacity;
       newOpacity = nOpacity;
    }
    void exec() {
        if (!m_content)
            return;
        m_content->setProperty("contentOpacity", newOpacity);
        m_content->update();
    }
    void unexec() {
        if (!m_content)
            return;
        m_content->setProperty("contentOpacity", previousOpacity);
        m_content->update();
    }
    QString name() const {
        return tr("Opacity command: ") + QString::number(previousOpacity) + " -> " + QString::number(newOpacity);
    }
};

//DONE
/* This commands manage the text of the TextContent */
class TextCommand: public AbstractCommand {
private:
    TextContent *m_content;
    QString m_previousText, m_newText;
public:
    TextCommand(TextContent *content, QString newText) :
            m_content(content), m_newText(newText) {
        m_previousText = content->toHtml();
    }
    void exec() {
        if (!m_content)
            return;
        m_content->setHtml(m_newText);
    }
    void unexec() {
        if (!m_content)
            return;
        m_content->setHtml(m_previousText);
    }
    void replaceContent(AbstractContent *oldContent,
            AbstractContent *newContent) {
        if (m_content == oldContent) {
            m_content = 0;
            m_content = dynamic_cast<TextContent *>(newContent);
        }
    }
    virtual AbstractContent* content() const
    {
      return m_content;
    }
    QString name() const {
        return tr("Text changed");
    }
};

class PerspectiveCommand : public AbstractCommand
{
private:
    QPointF m_previous, m_new;

public:
    PerspectiveCommand(AbstractContent* content, const QPointF& p, const QPointF &n) : m_previous(p), m_new(n)
    {
      m_content = content;
    }

    void exec() {
      m_content->setPerspective(m_new);
      qDebug() << "set perspective " << m_previous << " -> " << m_new;
    }

    void unexec() {
      m_content->setPerspective(m_previous);
      qDebug() << "set perspective " << m_new << " -> " << m_previous;
    }

    QString name() const {
        return tr("Perspective");
    }
};

//DONE
/* This commands manages transformations: rotations and scaling */
class TransformCommand: public AbstractCommand {
private:
    QTransform m_previous, m_new;

public:
    TransformCommand(AbstractContent *content, QTransform& previous_matrix,
            QTransform& new_matrix) :
            m_previous(previous_matrix), m_new(new_matrix) {
        m_content = content;
    }
    void exec() {
        m_content->setTransform(m_new);
    }
    void unexec() {
        m_content->setTransform(m_previous);
    }
    QString name() const {
        return tr("Transformation");
    }
};

//DONE
class RotateAndResizeCommand: public AbstractCommand {
private:
    const qreal m_pAngle, m_nAngle;
    const QRect m_pRect, m_nRect;
public:
    RotateAndResizeCommand(AbstractContent *content, const qreal pAngle,
            const qreal nAngle, const QRect pRect, const QRect nRect) :
            m_pAngle(pAngle), m_nAngle(nAngle), m_pRect(pRect), m_nRect(nRect) {
        m_content = content;
    }
    void exec() {
        m_content->setRotation(m_nAngle);
        m_content->resizeContents(m_nRect);
    }
    void unexec() {
        m_content->setRotation(m_pAngle);
        m_content->resizeContents(m_pRect);
    }
    QString name() const {
        return tr("Rotation and Resize");
    }
};

//DONE
/* This command manages movements of the content */
class MotionCommand: public AbstractCommand {
private:
    QPointF m_previous, m_newMotion;
public:
    MotionCommand(AbstractContent *content, QPointF previous, QPointF newMotion) :
            m_previous(previous), m_newMotion(newMotion) {
      m_content = content;
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
        QString previousStr = QString::number(m_previous.x())
                + QString::number(m_previous.y());
        QString newMotionStr = QString::number(m_newMotion.x())
                + QString::number(m_newMotion.y());
        return tr("Moved from %1 to %2").arg(previousStr, newMotionStr);
    }
};

//DONE
class DeleteContentCommand: public AbstractCommand {
private:
    Canvas *m_canvas;
    QDomElement m_contentElt;
    AbstractContent *m_oldContent;
public:
    DeleteContentCommand(AbstractContent *content, Canvas *canvas) :
            m_canvas(canvas) {
        m_oldContent = content;
        m_content = content;
    }

    void exec() {
        qDebug() << "DeleteContentCommand::exec";
        // Saves the adress of the current content to be able to replace it by the new one in the stack later.
        m_oldContent = m_content;

        // Get the content xml info, in order to recreate it correctly
        QDir t = QDir::currentPath();
        QDomDocument doc;
        m_contentElt = doc.createElement("content");
        m_content->toXml(m_contentElt, t); // XXX: why does it need a path ???

        m_canvas->deleteContent(m_content);
        // Content is deleted
        m_content = 0;
    }

    void unexec() {
      qDebug() << "DeleteContentCommand::unexec";
      // Recreate content from xml
      m_content = m_canvas->addContentFromXml(m_contentElt);
      if(m_content == 0)
      {
        qDebug() << "DeleteContentCommand: error reloading content";
        return;
      }
      else
      {
        // Replace content address in stack
        CommandStack::instance().replaceContent(m_oldContent, m_content);
      }
    }

    virtual void replaceContent(AbstractContent * /*oldContent*/, AbstractContent * /*newContent*/) {
    }

    virtual AbstractContent* content() const
    {
      return m_content;
    }

    QString name() const {
        return tr("Delete content");
    }
};

class NewContentCommand: public AbstractCommand {
private:
    Canvas *m_canvas;
    DeleteContentCommand *m_command;

public:
    NewContentCommand(Canvas *canvas, AbstractContent *content) {
        m_canvas = canvas;
        // Content is handled by DeleteContentCommand instead
        m_content = 0;
        m_command = new DeleteContentCommand(content, m_canvas);
    }
    void exec() {
      qDebug() << "NewContentCommand:exec: ";
      m_command->unexec();

    }
    void unexec() {
      qDebug() << "NewContentCommand::unexec";
      m_command->exec();
    }

    virtual void replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
      m_command->replaceContent(oldContent, newContent);
    }

    virtual AbstractContent* content() const
    {
      return m_command->content();
    }
    QString name() const {
        return tr("Add content");
    }
    QString description() const {
        return "";
    }
};

//DONE
/* This command manages movements of the content */
class FrameCommand: public AbstractCommand {
private:
    quint32 m_previousClass, m_newClass;
    bool m_previousMirror, m_newMirror;
public:
    FrameCommand(AbstractContent *content, quint32 newClass, bool mirrored) :
            m_newClass(newClass), m_newMirror(mirrored) {
        m_content = content;
        m_previousClass = content->frameClass();
        m_previousMirror = content->mirrored();
    }
    void exec() {
        m_content->setFrame(FrameFactory::createFrame(m_newClass));
        m_content->setMirrored(m_newMirror);
    }
    void unexec() {
        m_content->setFrame(FrameFactory::createFrame(m_previousClass));
        m_content->setMirrored(m_previousMirror);
    }

    QString name() const {
        return tr("Change frame/mirror");
    }
};

//DONE
/* Manage background content */
class BackgroundContentCommand: public AbstractCommand {
private:
    Canvas *m_canvas;
    AbstractContent *m_previousContent, *m_newContent;
public:
    BackgroundContentCommand(Canvas *desk, AbstractContent *previousContent,
            AbstractContent *newContent) :
            m_canvas(desk), m_previousContent(previousContent), m_newContent(
                    newContent) {
    }
    void exec() {
        m_canvas->setBackContent(m_newContent);
    }
    void unexec() {
        m_canvas->setBackContent(m_previousContent);
    }
    void replaceContent(AbstractContent *oldContent,
            AbstractContent *newContent) {
        if (m_previousContent == oldContent) {
            m_previousContent = newContent;
        } else if (m_newContent == oldContent) {
            m_newContent = newContent;
        }
    }
    QString name() const {
        return tr("Change background");
    }

};

//DONE
class BackgroundRatioCommand: public AbstractCommand {
private:
    Canvas *m_canvas;
    const Qt::AspectRatioMode m_previousRatio, m_newRatio;
public:
    BackgroundRatioCommand(Canvas *desk,
            const Qt::AspectRatioMode& previousRatio,
            const Qt::AspectRatioMode& newRatio) :
            m_canvas(desk), m_previousRatio(previousRatio), m_newRatio(newRatio) {
    }
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
class BackgroundModeCommand: public AbstractCommand {
private:
    Canvas *m_canvas;
    const Canvas::BackMode m_previousMode, m_newMode;
public:
    BackgroundModeCommand(Canvas *desk, const Canvas::BackMode& previousMode,
            const Canvas::BackMode& newMode) :
            m_canvas(desk), m_previousMode(previousMode), m_newMode(newMode) {
    }
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
class DecoTopBarCommand: public AbstractCommand {
    Canvas *m_canvas;
    QAction *m_action;
    bool m_state;

public:
    DecoTopBarCommand(Canvas *canvas, QAction* action, bool state) :
            m_canvas(canvas), m_action(action), m_state(state) {
    }
    void exec() {
        m_canvas->setTopBarEnabled(m_state);
        m_action->setChecked(m_state);
    }
    void unexec() {
        m_canvas->setTopBarEnabled(!m_state);
        m_action->setChecked(!m_state);
    }
    QString name() const {
        return tr("Top bar enabled/disabled");
    }
};

//DONE
class DecoBottomBarCommand: public AbstractCommand {
    Canvas *m_canvas;
    QAction *m_action;
    bool m_state;

public:
    DecoBottomBarCommand(Canvas *canvas, QAction *action, bool state) :
            m_canvas(canvas), m_action(action), m_state(state) {
    }
    void exec() {
        m_canvas->setBottomBarEnabled(m_state);
        m_action->setChecked(m_state);
    }
    void unexec() {
        m_canvas->setBottomBarEnabled(!m_state);
        m_action->setChecked(!m_state);
    }
    QString name() const {
        return tr("Bottom bar enabled/disabled");
    }
};

//DONE
class DecoTitleCommand: public AbstractCommand {
private:
    Canvas *m_canvas;
    QString m_previousText, m_newText;
public:
    DecoTitleCommand(Canvas *canvas, QString newText) :
            m_canvas(canvas), m_newText(newText) {
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
class ColorPickerCommand: public AbstractCommand {
    ColorPickerItem *m_cItem;
    const QColor m_pColor, m_nColor;

public:
    ColorPickerCommand(ColorPickerItem *item, const QColor pCol,
            const QColor nCol) :
            m_cItem(item), m_pColor(pCol), m_nColor(nCol) {
    }

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
class StackCommand: public AbstractCommand {
    int m_pZ, m_nZ;
public:
    StackCommand(AbstractContent *c, int pZ, int nZ) :
            m_pZ(pZ), m_nZ(nZ) {
        m_content = c;
    }

    void exec() {
        m_content->setZValue(m_nZ);
    }
    void unexec() {
        m_content->setZValue(m_pZ);
    }
    QString name() const {
        return tr("Stack order");
    }
};

//DONE
class ShapeCommand: public AbstractCommand {
    TextContent* m_content;
    QList<QPointF> m_pCps, m_nCps;
public:
    ShapeCommand(TextContent *c, const QList<QPointF>& pCps,
            const QList<QPointF>& nCps) :
            m_content(c), m_pCps(pCps), m_nCps(nCps) {
    }

    void exec() {
        if (!m_content)
            return;
        m_content->setControlPoints(m_nCps);
    }
    void unexec() {
        if (!m_content)
            return;
        m_content->setControlPoints(m_pCps);
    }
    virtual AbstractContent* content() const
    {
      return m_content;
    }
    void replaceContent(AbstractContent *oldContent,
            AbstractContent *newContent) {
        if (m_content == oldContent) {
            m_content = 0;
            m_content = dynamic_cast<TextContent *>(newContent);
        }
    }
};

#endif

