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
#include <QDebug>

/* This class keep a track of all the commands executed. It allows to do/undo each command
 * by calling AbstractCommand virtual functions.
 * It is a singleton, so it can be instancied only once. */
class CommandStack
{
    public :
        // Function to get the single instance of the class
        static CommandStack & instance();

        // Add the command in the stack, and execute it
        void doCommand(AbstractCommand *command)
        {
            qDebug() << "do command << " << command->name();
            m_undoStack.push_back(command);
            command->exec();
        }
        // Add the command in the stack, but do not execute it
        // (useful for letting Qt manage the moves for exemple)
        void addCommand(AbstractCommand *command)
        {
            qDebug() << "add command << " << command->name();
            m_undoStack.push_back(command);
        }

        void undoLast()
        {
            if(m_undoStack.size() == 0) return;
            AbstractCommand * command = m_undoStack.takeLast();
            command->unexec();
            m_redoStack.push_back(command);
        }
        void redoLast()
        {
            if(m_redoStack.size() == 0) return;
            AbstractCommand *command = m_redoStack.takeLast();
            command->exec();
            m_undoStack.push_back(command);
        }

    private :
        // Private constructor, so it is impossible to create an instant without using instance()
        CommandStack() {}

        // Unable copy
        CommandStack( const CommandStack & );
        CommandStack & operator =( const CommandStack & );

    protected:
        QList<AbstractCommand *> m_undoStack;
        QList<AbstractCommand *> m_redoStack;
};

#endif
