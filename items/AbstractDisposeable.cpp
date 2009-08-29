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

#include "AbstractDisposeable.h"

AbstractDisposeable::AbstractDisposeable(QGraphicsItem * parent, bool fadeIn)
#if QT_VERSION >= 0x040600
    : QGraphicsObject(parent)
#else
    : QGraphicsItem(parent)
#endif
{
    if (fadeIn) {
#if QT_VERSION >= 0x040600
        // fade in animation
        QPropertyAnimation * ani = new QPropertyAnimation(this, "opacity");
        ani->setEasingCurve(QEasingCurve::OutCubic);
        ani->setDuration(300);
        ani->setStartValue(0.0);
        ani->setEndValue(1.0);
        ani->start(QPropertyAnimation::DeleteWhenStopped);
#endif
        show();
    }
}

void AbstractDisposeable::dispose()
{
#if QT_VERSION >= 0x040600
    // fade out animation, then delete
    QPropertyAnimation * ani = new QPropertyAnimation(this, "opacity");
    connect(ani, SIGNAL(finished()), this, SLOT(deleteLater()));
    ani->setEasingCurve(QEasingCurve::OutCubic);
    ani->setDuration(300);
    ani->setEndValue(0.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    // delete this now
    deleteLater();
#endif
}
