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

/* This class keep a track of all the commands executed. It allows to exec/unexec each command
 * by calling AbstractCommand virtual functions.
 * It is a singleton, so it can be instancied only once. */
class CommandStack
{
    public :
        // Function to get the single instance of the class
        static CommandStack & instance();

        // Add the command in the stack, and execute it
        void doCommand(AbstractCommand *command);
        // Add the command in the stack, but do not execute it
        // (useful for letting Qt manage the moves for exemple)
        void addCommand(AbstractCommand *command);

        // Recursively map and replace old (deleted) content with their new
        // (recreated) counterpart
        // This is used when recreating previously deleted content to update the
        // pointer addresses in the whole command stack
        void replaceContent(const QList<void *>& oldContent, const QList<AbstractContent *>& newContent);

        void undoLast();
        void redoLast();

    private :
        // Private constructor, so it is impossible to create an instant without using instance()
        CommandStack() {}

        // Unable copy
        CommandStack( const CommandStack & );
        CommandStack & operator =( const CommandStack & );


        // Maps old addresses to new addresses in commands
        void replaceContent(const QList<AbstractCommand *>& commands, const void * oldContent, AbstractContent* newContent, QMap<AbstractCommand *, AbstractContent *>& newCommandContent);
        void replaceContent(const QList<AbstractCommand *>& commands, const QList<void *>& oldContents, const QList<AbstractContent *>& newContents, QMap<AbstractCommand *, AbstractContent *>& newCommandContent);

        // Prevents undo and redo to be running simultaneously
        QMutex m_mutex;

    protected:
        QList<AbstractCommand *> m_undoStack;
        QList<AbstractCommand *> m_redoStack;
};

#endif
