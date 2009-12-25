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
#include <QDebug>

CommandStack & CommandStack::instance()
{
    static CommandStack instance; // instance unique cachée dans la fonction. Ne pas oublier le static !
    return instance;
}


void CommandStack::doCommand(AbstractCommand *command)
{
    qDebug() << "do command << " << command->name();
    m_undoStack.push_back(command);
    command->exec();
}

void CommandStack::addCommand(AbstractCommand *command)
{
    qDebug() << "add command << " << command->name();
    m_undoStack.push_back(command);
}

void CommandStack::undoLast()
{
    if(m_undoStack.isEmpty()) return;
    AbstractCommand * command = m_undoStack.takeLast();
    qDebug() << "undo command " << command->name();
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

void CommandStack::changeContent(AbstractContent *pC, AbstractContent *nC)
{
    foreach (AbstractCommand *c, m_undoStack) {
        if(c->content() == pC) {
            qDebug() << "Command " << c->name() << " set content : " << nC << " instead of " << pC;
            c->setContent(nC);
        }
    }
    foreach (AbstractCommand *c, m_redoStack) {
        if(c->content() == pC)
            qDebug() << "Command " << c->name() << " set content : " << nC << " instead of " << pC;
            c->setContent(nC);
    }
}
