/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __AbstractDisposeable_h__
#define __AbstractDisposeable_h__

#include <QtGlobal>
#if QT_VERSION >= 0x040600
#include <QGraphicsObject>
#include <QPropertyAnimation>
#else
#include <QObject>
#include <QGraphicsItem>
#endif

/**
    \class AbstractDisposeable
    Base class of 'disposeable' items (items you can call 'dispose' on them and
    forget about them.
*/
#if QT_VERSION >= 0x040600
class AbstractDisposeable : public QGraphicsObject
#else
class AbstractDisposeable : public QObject, public QGraphicsItem
#endif
{
    public:
        AbstractDisposeable(QGraphicsItem * parent = 0, bool fadeIn = false);
        virtual ~AbstractDisposeable() {}

        // reimplement this to add a custom deletion behavior
        virtual void dispose();
};

#endif
