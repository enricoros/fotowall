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
bool CommandStack::doCommand(AbstractCommand* command) {
    if (command == 0)
        return false;
    qDebug() << "[CommandStack] exec command << " << command->name();
    if(addCommand(command)) {
        command->exec();
    }
    return true;
}

bool CommandStack::addCommandSafe(AbstractCommand* command) {
    // Only add command if it is valid and we are not currently undoing/redoing
    // actions
    if (command == nullptr || isUndoInProgress() || isRedoInProgress()) {
        return false;
    }
    // Only add GroupedCommand if they contain at least one element
    else if (GroupedCommands* c = dynamic_cast<GroupedCommands*>(command)) {
        if (c->size() == 0)
            return false;
    }
    qDebug() << "[CommandStack] add command: \n" << command->name() << "\n " << command->description();

    // Clear redo stack when adding new command
    // NOTE: Could be made configurable to allow redo to occur after
    // modifications have been made to the content. If so, one would need
    // to take care of filtering out deleted content.
    qDeleteAll(m_redoStack);
    m_redoStack.clear();

    m_undoStack.push_back(command);
    return true;
}

bool CommandStack::addCommand(AbstractCommand* command) {
    bool ret = addCommandSafe(command);
    if (!ret) {
        if (command != 0)
            delete command;
    }
    return ret;
}

bool CommandStack::isUndoInProgress() const {
    return m_undoInProgress;
}
bool CommandStack::isRedoInProgress() const {
    return m_redoInProgress;
}

void CommandStack::undoLast() {
    // Do not run until redo finishes
    QMutexLocker lock(&m_mutex);

    if (m_undoStack.isEmpty())
        return;

    m_undoInProgress = true;
    AbstractCommand* command = m_undoStack.last();
    qDebug() << "[CommandStack] undo command " << command->name();
    command->unexec();
    m_redoStack.push_back(command);
    m_undoStack.takeLast();
    m_undoInProgress = false;
    lock.unlock();
}

void CommandStack::redoLast() {
    // Do not run until undo finishes
    QMutexLocker lock(&m_mutex);
    if (m_redoStack.isEmpty())
        return;

    m_redoInProgress = true;
    AbstractCommand* command = m_redoStack.last();
    qDebug() << "[CommandStack] redo command " << command->name();
    command->exec();
    m_undoStack.push_back(command);
    m_redoStack.takeLast();
    m_redoInProgress = false;
    lock.unlock();
}
