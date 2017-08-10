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

CommandStack & CommandStack::instance()
{
    static CommandStack instance; // instance unique cachée dans la fonction. Ne pas oublier le static !
    return instance;
}


void CommandStack::doCommand(AbstractCommand *command)
{
    qDebug() << "exec command << " << command->name();
    addCommand(command);
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
    foreach (AbstractCommand *command, m_undoStack) {
        qDebug() << "CommandStack: updating undo stack"<< command->name();
        command->replaceContent(oldContent, newContent);
    }
    foreach (AbstractCommand *command, m_redoStack) {
        qDebug() << "CommandStack: updating redo stack"<< command->name();
        command->replaceContent(oldContent, newContent);
    }
}

void CommandStack::undoLast()
{
    if(m_undoStack.isEmpty()) return;
    AbstractCommand * command = m_undoStack.takeLast();
    qDebug() << "unexec command " << command->name();
    if(command != 0) {
        command->unexec();
        m_redoStack.push_back(command);
    }
}

void CommandStack::redoLast()
{
    if(m_redoStack.isEmpty()) return;
    AbstractCommand *command = m_redoStack.takeLast();
    qDebug() << "redo command " << command->name();
    if(command != 0) {
        command->exec();
        m_undoStack.push_back(command);
    }
}
