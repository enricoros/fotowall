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
#include "GroupedCommands.h"
#include <QDebug>
#include <QMutexLocker>

CommandStack & CommandStack::instance()
{
    static CommandStack instance; // instance unique cachée dans la fonction. Ne pas oublier le static !
    return instance;
}


void CommandStack::doCommand(AbstractCommand *command)
{
    addCommand(command);
    qDebug() << "exec command << " << command->name();
    command->exec();
}

void CommandStack::addCommand(AbstractCommand *command)
{
    // Check if GroupedCommand contains at least one element
    if(GroupedCommands * c = dynamic_cast<GroupedCommands *>(command))
    {
      if(c->size() == 0) return;
    }
    qDebug() << "add command << " << command->name();
    m_undoStack.push_back(command);
}

void CommandStack::replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
    qDebug() << "CommandStack: updating content in stack";
    foreach (AbstractCommand *command, m_undoStack) {
      qDebug() << "command: " << command->name() << ":\n"
          << "content    : " << command->content()
          <<"\noldContent: " << oldContent
          <<"\nnewContent: " << newContent;
        command->replaceContent(oldContent, newContent);
    }
    foreach (AbstractCommand *command, m_redoStack) {
      qDebug() << "command: " << command->name() << ":\n"
          << "content    : " << command->content()
          <<"\noldContent: " << oldContent
          <<"\nnewContent: " << newContent;
        command->replaceContent(oldContent, newContent);
    }
}

void CommandStack::undoLast()
{
  QMutexLocker lock(&m_mutex);
  // Do not run if redo is in progress
  // XXX undo must wait on redo to complete
  if(m_undoStack.isEmpty()) return;
  AbstractCommand * command = m_undoStack.last();
  qDebug() << "undo command " << command->name();
  qDebug() << "redo stack size: " << m_redoStack.size();
  if(command != 0) {
    command->unexec();
    m_redoStack.push_back(command);
  }
  else
  {
    qDebug() << "undoLast: Error: null command!";
  }
  m_undoStack.takeLast();
  qDebug() << "undo finished";
  lock.unlock();
}

void CommandStack::redoLast()
{
  QMutexLocker lock(&m_mutex);
  // Do not run if undo is in progress.
  // redo must wait on undo to complete
  if(m_redoStack.isEmpty()) return;
  AbstractCommand *command = m_redoStack.last();
  qDebug() << "redo command " << command->name();
  qDebug() << "undo stack size: " << m_undoStack.size();
  if(command != 0) {
    command->exec();
    m_undoStack.push_back(command);
  }
  else
  {
    qDebug() << "redoLast: Error: null command!";
  }
  m_redoStack.takeLast();
  qDebug() << "redo finished";
  lock.unlock();
}
