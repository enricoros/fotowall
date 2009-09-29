/***************************************************************************
 *                                                                         *
 *   This file is part of the WordCloud project,                           *
 *       http://www.enricoros.com/opensource/wordcloud                     *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __WordItem_h__
#define __WordItem_h__

#include <QAbstractGraphicsShapeItem>
#include "WordDefs.h"

namespace WordCloud {

    /// \brief A word, displayable and containing path information
    class WordItem : public QAbstractGraphicsShapeItem
    {
        public:
            WordItem(const Word & word, const QFont & font, double rotation,
                     int minCount, int maxCount, QGraphicsItem * parent = 0);

            QRectF tmpPlacedRect;

            // get the word path
            inline QPainterPath path() const { return m_path; }
            QPainterPath strokedPath() const;

            // ::QGraphicsItem
            inline QRectF boundingRect() const { return m_boudingRect; }
            void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

        private:
            WordItem();
            void regeneratePath();

            QFont m_font;
            double m_rotation;
            Word m_word;

            QPainterPath m_path;
            QRectF m_boudingRect;
    };

    /// convenience alias
    typedef QList<WordItem *> WordItemList;

    /// how to place the words
    enum Placement {
        Horizontal = 0, Vertical = 1, MostlyHorizontal = 2,
        MostlyVertical = 3, RandomRotation = 4, RandomPosition = 5,
        Placement_COUNT = 6
    };

    /// how to update screen while processing
    enum BusyMode {
        Block, ShowPlacement, ShowProgress
    };
}

#endif
