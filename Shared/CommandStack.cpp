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

#include "CommandStack.h"
#include "Canvas/AbstractContent.h"
#include "Commands.h"
#include "GroupedCommands.h"
#include <QDebug>
#include <QMutexLocker>

CommandStack& CommandStack::instance() {
    static CommandStack instance; // instance unique cachée dans la fonction. Ne pas oublier le static !
    return instance;
}

void CommandStack::doCommand(AbstractCommand* command) {
    if (command == 0)
        return;
    addCommand(command);
    qDebug() << "[CommandStack] exec command << " << command->name();
    command->exec();
}

void CommandStack::addCommand(AbstractCommand* command) {
    if (command == 0)
        return;

    // Clear redo stack when adding new command
    // NOTE: Could be made configurable to allow redo to occur after
    // modifications have been made to the content. If so, one would need
    // to take care of filtering out deleted content.
    qDeleteAll(m_redoStack);
    m_redoStack.clear();

    // Only add GroupedCommand if they contain at least one element
    if (GroupedCommands* c = dynamic_cast<GroupedCommands*>(command)) {
        if (c->size() == 0)
            return;
    }
    qDebug() << "[CommandStack] add command << " << command->name();
    m_undoStack.push_back(command);
}

void CommandStack::replaceContent(const QList<AbstractCommand*>& commands, const void* oldContent, AbstractContent* newContent, QMap<AbstractCommand*, AbstractContent*>& newCommandContent) {
    // Find commands containing this content
    foreach (AbstractCommand* c, commands) {
        // XXX make this recursive
        if (GroupedCommands* gc = dynamic_cast<GroupedCommands*>(c)) {
            replaceContent(gc->commands(), oldContent, newContent, newCommandContent);
        } else if (c->hasContent(oldContent)) {
            newCommandContent[c] = newContent;
        }
    }
}

void CommandStack::replaceContent(const QList<AbstractCommand*>& commands, const QList<void*>& oldContents, const QList<AbstractContent*>& newContents, QMap<AbstractCommand*, AbstractContent*>& newCommandContent) {
    for (int i = 0; i < oldContents.size(); ++i) {
        void* oldContent = (void*)oldContents[i];
        AbstractContent* newContent = newContents[i];
        replaceContent(commands, oldContent, newContent, newCommandContent);
    }
}

void CommandStack::replaceContent(const QList<void*>& oldContents, const QList<AbstractContent*>& newContents) {
    qDebug() << "[CommandStack] updating content in stack";

    // Maps old (deleted) content addresses to new content addresses
    QMap<AbstractCommand*, AbstractContent*> newCommandContent;
    replaceContent(m_undoStack, oldContents, newContents, newCommandContent);
    replaceContent(m_redoStack, oldContents, newContents, newCommandContent);

    // Replace old content with new mapped addresses
    QMapIterator<AbstractCommand*, AbstractContent*> i(newCommandContent);
    while (i.hasNext()) {
        i.next();
        i.key()->setContent(i.value());
    }
}

void CommandStack::undoLast() {
    // Do not run until redo finishes
    QMutexLocker lock(&m_mutex);

    if (m_undoStack.isEmpty())
        return;
    AbstractCommand* command = m_undoStack.last();
    qDebug() << "[CommandStack] undo command " << command->name();
    command->unexec();
    m_redoStack.push_back(command);
    m_undoStack.takeLast();
    lock.unlock();
}

void CommandStack::redoLast() {
    // Do not run until undo finishes
    QMutexLocker lock(&m_mutex);

    if (m_redoStack.isEmpty())
        return;
    AbstractCommand* command = m_redoStack.last();
    qDebug() << "[CommandStack] redo command " << command->name();
    command->exec();
    m_undoStack.push_back(command);
    m_redoStack.takeLast();
    lock.unlock();
}
