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

#ifndef __CommandStack__
#define __CommandStack__

#include "AbstractCommand.h"
#include <QList>
#include <QMutex>
#include <QMutexLocker>

/*
 * This class keep a track of all the commands executed. It allows to exec/unexec each command
 * by calling AbstractCommand virtual functions.
 */
class CommandStack {
  public:
    CommandStack() = default;

    // delete copy constructor and assignment operator
    CommandStack(const CommandStack&) = delete;
    CommandStack& operator=(const CommandStack&) = delete;
    // delete move constructor and assignment operator
    CommandStack(CommandStack&&) = delete;
    CommandStack& operator=(CommandStack&&) = delete;

    // Add the command in the stack, and execute it
    bool doCommand(AbstractCommand* command);
    // Add the command in the stack, but do not execute it
    // (useful for letting Qt manage the moves for exemple)
    bool addCommand(AbstractCommand* command);

    void undoLast();
    void redoLast();
    bool isUndoInProgress() const;
    bool isRedoInProgress() const;

  private:

    bool addCommandSafe(AbstractCommand*);

    // Prevents undo and redo to be running simultaneously
    QMutex m_mutex;

    // True when undo/redo in progress
    bool m_undoInProgress = false;
    bool m_redoInProgress = false;

  protected:
    QList<AbstractCommand*> m_undoStack;
    QList<AbstractCommand*> m_redoStack;
};

#endif
