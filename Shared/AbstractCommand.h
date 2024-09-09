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

#ifndef __AbstractCommand__
#define __AbstractCommand__

#include <QDebug>
#include <QObject>
#include <QString>

class AbstractContent;

/* This class is used to implement the command pattern.
 * It provides pure virtual function to do/undo an action. */
class AbstractCommand : public QObject {
  protected:
    QList<AbstractContent*> m_content;

  public:
    AbstractCommand() {}
    AbstractCommand(AbstractContent* content) {
        m_content.push_back(content);
    }
    AbstractCommand(const QList<AbstractContent*>& content)
        : m_content(content) {}

    virtual void exec() = 0;
    virtual void unexec() = 0;
    virtual QString name() const {
        return QString();
    }
    virtual QString description() const {
        return QString();
    }
};

#endif
