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
#include "Shared/ColorPickerItem.h"
#include "Shared/CommandStack.h"
#include "Shared/GroupedCommands.h"
#include "Shared/PictureEffect.h"

#include "App/CanvasAppliance.h"
#include "Canvas/AbstractContent.h"
#include "Canvas/Canvas.h"
#include "Canvas/PictureContent.h"
#include "Canvas/TextContent.h"
#include "Canvas/WebcamContent.h"
#include "Canvas/WordcloudContent.h"
#include "Frames/FrameFactory.h"
#include <QAction>
#include <QDebug>
#include <QStringList>
#include <QUrl>

/**
 * TODO
 * - Type of canvas (cd, dvd...)
 * - CanvasViewContent
 */

//DONE
class EffectCommand : public AbstractCommand {
  private:
    QList<PictureEffect> m_previousEffects;
    PictureEffect m_newEffect;
    QRect m_previousSize;

  public:
    EffectCommand(PictureContent* content, const PictureEffect& effect) : AbstractCommand(content), m_newEffect(effect) {
        m_previousEffects = content->effects();
        m_previousSize = content->contentRect();
    }
    void exec() {
        PictureContent* c = dynamic_cast<PictureContent*>(m_content[0]);
        if (!c)
            return;
        m_previousSize = c->contentRect();
        c->addEffect(m_newEffect);
    }
    void unexec() {
        PictureContent* c = dynamic_cast<PictureContent*>(m_content[0]);
        if (!c)
            return;

        c->addEffect(PictureEffect::ClearEffects);
        foreach (PictureEffect effect, m_previousEffects) {
            c->addEffect(effect);
        }
        c->resizeContents(m_previousSize);
    }
    QString name() const {
        return tr("New effects");
    }
};

class OpacityCommand : public AbstractCommand {
  private:
    qreal m_pOpacity;
    qreal m_nOpacity;

  public:
    OpacityCommand(AbstractContent* content, qreal pOpacity, qreal nOpacity)
        : AbstractCommand(content), m_pOpacity(pOpacity), m_nOpacity(nOpacity) {
    }
    void exec() {
        AbstractContent* c = m_content[0];
        if (!c)
            return;
        c->setProperty("contentOpacity", m_nOpacity);
        c->update();
    }
    void unexec() {
        AbstractContent* c = m_content[0];
        if (!c)
            return;
        c->setProperty("contentOpacity", m_pOpacity);
        c->update();
    }
    QString name() const {
        return tr("Opacity command: ") + QString::number(m_pOpacity) + " -> " + QString::number(m_nOpacity);
    }
};

//DONE
/* This commands manage the text of the TextContent */
class TextCommand : public AbstractCommand {
  private:
    QString m_previousText, m_newText;

  public:
    TextCommand(TextContent* content, QString newText)
        : AbstractCommand(content), m_newText(newText) {
        m_previousText = content->toHtml();
    }
    void exec() {
        TextContent* c = dynamic_cast<TextContent*>(m_content[0]);
        if (!c)
            return;
        c->setHtml(m_newText);
    }
    void unexec() {
        TextContent* c = dynamic_cast<TextContent*>(m_content[0]);
        if (!c)
            return;
        c->setHtml(m_previousText);
    }
    QString name() const {
        return tr("Text changed");
    }
};

class PerspectiveCommand : public AbstractCommand {
  private:
    QPointF m_previous, m_new;

  public:
    PerspectiveCommand(AbstractContent* content, const QPointF& p, const QPointF& n)
        : AbstractCommand(content), m_previous(p), m_new(n) {
    }

    void exec() {
        AbstractContent* c = m_content[0];
        c->setPerspective(m_new);
        qDebug() << "set perspective " << m_previous << " -> " << m_new;
    }

    void unexec() {
        AbstractContent* c = m_content[0];
        c->setPerspective(m_previous);
        qDebug() << "set perspective " << m_new << " -> " << m_previous;
    }

    QString name() const {
        return tr("Perspective");
    }
};

//DONE
/* This commands manages transformations: rotations and scaling */
class TransformCommand : public AbstractCommand {
  private:
    QTransform m_previous, m_new;

  public:
    TransformCommand(AbstractContent* content, QTransform& previous_matrix,
                     QTransform& new_matrix)
        : AbstractCommand(content), m_previous(previous_matrix), m_new(new_matrix) {
    }
    void exec() {
        m_content[0]->setTransform(m_new);
    }
    void unexec() {
        m_content[0]->setTransform(m_previous);
    }
    QString name() const {
        return tr("Transformation");
    }
};

//DONE
class RotateAndResizeCommand : public AbstractCommand {
  private:
    const qreal m_pAngle, m_nAngle;
    const QRect m_pRect, m_nRect;

  public:
    RotateAndResizeCommand(AbstractContent* content, const qreal pAngle,
                           const qreal nAngle, const QRect pRect, const QRect nRect)
        : AbstractCommand(content), m_pAngle(pAngle), m_nAngle(nAngle), m_pRect(pRect), m_nRect(nRect) {}
    void exec() {
        m_content[0]->setRotation(m_nAngle);
        m_content[0]->resizeContents(m_nRect);
    }
    void unexec() {
        m_content[0]->setRotation(m_pAngle);
        m_content[0]->resizeContents(m_pRect);
    }
    QString name() const {
        return tr("Rotation and Resize");
    }
};

//DONE
/* This command manages movements of the content */
class MotionCommand : public AbstractCommand {
  private:
    QPointF m_previous, m_newMotion;

  public:
    MotionCommand(AbstractContent* content, QPointF previous, QPointF newMotion)
        : AbstractCommand(content), m_previous(previous), m_newMotion(newMotion) {
    }
    void exec() {
        m_content[0]->setPos(m_newMotion);
    }
    void unexec() {
        m_content[0]->setPos(m_previous);
    }
    void setPrevious(const QPointF& prev) {
        m_previous = prev;
    }
    void setCurrent(const QPointF& current) {
        m_newMotion = current;
    }
    QString name() const {
        return tr("Move");
    }
    QString description() const {
        QString previousStr = "(" + QString::number(m_previous.x()) + ", " + QString::number(m_previous.y()) + ")";
        QString newMotionStr = "(" + QString::number(m_newMotion.x()) + ", " + QString::number(m_newMotion.y()) + ")";
        return tr("Moved from %1 to %2").arg(previousStr, newMotionStr);
    }
};

//DONE
/**
 * @brief DeleteContentCommand handles the deletion and reloading of contents
 *
 * - To reduce memory consumption, content deletion effectively removes the content from memory.
 *   The content is saved as XML for future reloading.
 * - Reloading is done by assigning memory for a new content, and loading it
 *   from the XML backup. This new content, having been assigned a new address
 *   different from the one previously used in the command stack, it is
 *   necessary to swap the old and new address in the whole command stack.
 */
class DeleteContentCommand : public AbstractCommand {
  private:
    QList<const void*> oldContents;
    QList<QDomElement> m_contentElts;
    Canvas* m_canvas;

  public:
    DeleteContentCommand(const QList<AbstractContent*>& contents, Canvas* canvas)
        : AbstractCommand(contents), m_canvas(canvas) {
        foreach (AbstractContent* c, contents) {
            oldContents.push_back((void*)c);
        }
    }

    void exec() {
        qDebug() << "DeleteContentCommand::exec";
        m_contentElts.clear();
        oldContents.clear();
        foreach (AbstractContent* content, m_content) {
            qDebug() << " deleting: " << (void*)content;
            // Get the content xml info, in order to recreate it correctly
            QDir t = QDir::currentPath();
            QDomDocument doc;
            QDomElement contentElt = doc.createElement("content");
            content->toXml(contentElt, t); // XXX: why does it need a path ???
            m_contentElts.push_back(contentElt);

            oldContents.push_back((void*)content);
            // Actually delete the content
            m_canvas->deleteContent(content);
        }
    }

    void unexec() {
        qDebug() << (void*)this << "DeleteContentCommand::unexec";
        QList<AbstractContent*> newContents;
        foreach (const QDomElement& contentElt, m_contentElts) {
            // Recreate content from xml
            AbstractContent* content = m_canvas->addContentFromXml(contentElt);
            // Content is restored with a new address now
            newContents.push_back(content);
        }
        qDebug() << "content created";
        qDebug() << oldContents.size();

        qDebug() << "oldcontents0 " << (void*)oldContents[0];
        qDebug() << "newcontents0 " << (void*)newContents[0];
        // Now replace old content addresses with new content addresses in stack
        CommandStack::instance().replaceContent(oldContents, newContents);
    }

    QString name() const {
        return tr("Delete content");
    }
};

/**
 * @brief NewContentCommand handles the creation/removal of content.
 * It delegates most of its functionalities to the opposite
 * DeleteContentCommand.
 */
class NewContentCommand : public AbstractCommand {
  private:
    DeleteContentCommand* m_command;

  public:
    NewContentCommand(AbstractContent* content, Canvas* canvas) {
        QList<AbstractContent*> contents;
        contents.push_back(content);
        init(contents, canvas);
    }

    NewContentCommand(const QList<AbstractContent*>& contents, Canvas* canvas) {
        init(contents, canvas);
    }
    void init(const QList<AbstractContent*>& contents, Canvas* canvas) {
        m_command = new DeleteContentCommand(contents, canvas);
    }
    void exec() {
        qDebug() << "NewContentCommand:exec: ";
        m_command->unexec();
    }
    void unexec() {
        qDebug() << "NewContentCommand::unexec";
        m_command->exec();
    }

    virtual bool replaceContent(const QList<const void*> old, const QList<AbstractContent*> content) {
        return m_command->replaceContent(old, content);
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
class FrameCommand : public AbstractCommand {
  private:
    quint32 m_previousClass, m_newClass;
    bool m_previousMirror, m_newMirror;

  public:
    FrameCommand(AbstractContent* content, quint32 newClass, bool mirrored)
        : AbstractCommand(content), m_newClass(newClass), m_newMirror(mirrored) {
        m_previousClass = content->frameClass();
        m_previousMirror = content->mirrored();
    }
    void exec() {
        m_content[0]->setFrame(FrameFactory::createFrame(m_newClass));
        m_content[0]->setMirrored(m_newMirror);
    }
    void unexec() {
        m_content[0]->setFrame(FrameFactory::createFrame(m_previousClass));
        m_content[0]->setMirrored(m_previousMirror);
    }

    QString name() const {
        return tr("Change frame/mirror");
    }
};

//DONE
/* Manage background content */
class BackgroundContentCommand : public AbstractCommand {
  private:
    Canvas* m_canvas;

  public:
    BackgroundContentCommand(Canvas* canvas, AbstractContent* previousContent,
                             AbstractContent* newContent) : m_canvas(canvas) {
        m_content.push_back(previousContent);
        m_content.push_back(newContent);
    }
    void exec() {
        m_canvas->setBackContent(m_content[1]);
    }
    void unexec() {
        m_canvas->setBackContent(m_content[0]);
    }
    QString name() const {
        return tr("Change background");
    }
};

//DONE
class BackgroundRatioCommand : public AbstractCommand {
  private:
    Canvas* m_canvas;
    const Qt::AspectRatioMode m_previousRatio, m_newRatio;

  public:
    BackgroundRatioCommand(Canvas* canvas,
                           const Qt::AspectRatioMode& previousRatio,
                           const Qt::AspectRatioMode& newRatio) : m_canvas(canvas), m_previousRatio(previousRatio), m_newRatio(newRatio) {
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
class BackgroundModeCommand : public AbstractCommand {
  private:
    Canvas* m_canvas;
    const Canvas::BackMode m_previousMode, m_newMode;

  public:
    BackgroundModeCommand(Canvas* desk, const Canvas::BackMode& previousMode,
                          const Canvas::BackMode& newMode) : m_canvas(desk), m_previousMode(previousMode), m_newMode(newMode) {
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
class DecoTopBarCommand : public AbstractCommand {
    Canvas* m_canvas;
    QAction* m_action;
    bool m_state;

  public:
    DecoTopBarCommand(Canvas* canvas, QAction* action, bool state) : m_canvas(canvas), m_action(action), m_state(state) {
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
class DecoBottomBarCommand : public AbstractCommand {
    Canvas* m_canvas;
    QAction* m_action;
    bool m_state;

  public:
    DecoBottomBarCommand(Canvas* canvas, QAction* action, bool state) : m_canvas(canvas), m_action(action), m_state(state) {
    }
    void exec() {
        m_canvas->setBottomBarEnabled(m_state);
        m_action->setChecked(m_state);
    }
    void unexec() {
        qDebug() << "canvas: " << (void*)m_canvas;
        qDebug() << "action: " << (void*)m_action;
        m_canvas->setBottomBarEnabled(!m_state);
        m_action->setChecked(!m_state);
    }
    QString name() const {
        return tr("Bottom bar enabled/disabled");
    }
};

//DONE
class DecoTitleCommand : public AbstractCommand {
  private:
    Canvas* m_canvas;
    QString m_previousText, m_newText;

  public:
    DecoTitleCommand(Canvas* canvas, QString newText) : m_canvas(canvas), m_newText(newText) {
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
    ColorPickerItem* m_cItem;
    const QColor m_pColor, m_nColor;

  public:
    ColorPickerCommand(ColorPickerItem* item, const QColor pCol,
                       const QColor nCol) : m_cItem(item), m_pColor(pCol), m_nColor(nCol) {
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
class StackCommand : public AbstractCommand {
    int m_pZ, m_nZ;

  public:
    StackCommand(AbstractContent* c, int pZ, int nZ) : m_pZ(pZ), m_nZ(nZ) {
        m_content.push_back(c);
    }

    void exec() {
        m_content[0]->setZValue(m_nZ);
    }
    void unexec() {
        m_content[0]->setZValue(m_pZ);
    }
    QString name() const {
        return tr("Stack order");
    }
};

//DONE
class ShapeCommand : public AbstractCommand {
    QList<QPointF> m_pCps, m_nCps;

  public:
    ShapeCommand(TextContent* c, const QList<QPointF>& pCps,
                 const QList<QPointF>& nCps) : AbstractCommand(c), m_pCps(pCps), m_nCps(nCps) {
    }

    void exec() {
        TextContent* c = dynamic_cast<TextContent*>(m_content[0]);
        if (!c)
            return;
        c->setControlPoints(m_nCps);
    }
    void unexec() {
        TextContent* c = dynamic_cast<TextContent*>(m_content[0]);
        if (!c)
            return;
        c->setControlPoints(m_pCps);
    }
    QString name() const {
        return tr("Text Shape");
    }
};

class ProjectModeCommand : public AbstractCommand {
    CanvasAppliance* m_app;
    int m_pMode, m_nMode;

  public:
    ProjectModeCommand(CanvasAppliance* app, const int pMode, const int nMode)
        : m_app(app), m_pMode(pMode), m_nMode(nMode) {}

    virtual void exec() {
        m_app->setProjectMode(m_nMode);
    }

    virtual void unexec() {
        m_app->setProjectMode(m_pMode);
    }
    virtual QString name() const {
        return tr("Project Mode");
    }
    virtual QString description() const {
        return tr("Changed project mode from %1 to %2").arg(QString::number(m_pMode), QString::number(m_nMode));
    }
};

class FxCommand : public AbstractCommand {
    int m_pIndex, m_nIndex;

  public:
    FxCommand(AbstractContent* content, int pIndex, int nIndex)
        : AbstractCommand(content), m_pIndex(pIndex), m_nIndex(nIndex) {
    }
    virtual void exec() {
        m_content[0]->setFxIndex(m_nIndex);
    }

    virtual void unexec() {
        m_content[0]->setFxIndex(m_pIndex);
    }
    virtual QString name() const {
        return tr("Content Fx");
    }
    virtual QString description() const {
        return QString();
    }
};

#endif
