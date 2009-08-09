
/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
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

#ifndef __BezierTextProperties_h__
#define __BezierTextProperties_h__

#include "AbstractProperties.h"
class QListWidgetItem;
class BezierTextContent;
class Bezier;


class BezierTextProperties : public AbstractProperties {
    public:
        BezierTextProperties(BezierTextContent * textContent, QGraphicsItem * parent = 0);
        ~BezierTextProperties();

    protected:
        // ::AbstractProperties
        void closing();

    private:
        BezierTextContent * m_bezierTextContent;
        Bezier* m_bezierWidget;
};

#endif
