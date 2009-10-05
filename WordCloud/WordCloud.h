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

#ifndef __WordCloud_h__
#define __WordCloud_h__

#include <QObject>
#include "WordDefs.h"
#include "WordItem.h"

namespace WordCloud {

    class Cloud : public QObject
    {
        Q_OBJECT
        public:
            Cloud(QObject * parent = 0);

            // set/take word items !!
            void newCloud(const WordList & word);
            void regenCloud();
            void randomCloud();

            // set the scene where to display items
            void setScene(QGraphicsScene * scene);
            QGraphicsScene * scene() const;
            void removeFromScene();

            void setPlacement(WordCloud::Placement);
            WordCloud::Placement placement() const;

            void setAccurate(bool accurate);
            bool accurate() const;

            void setAppearance(const WordCloud::Appearance &);
            WordCloud::Appearance appearance() const;

            void setBusyMode(WordCloud::BusyMode mode);
            WordCloud::BusyMode busyMode() const;

        private:
            void process();
            QGraphicsScene * m_scene;
            WordItemList m_wordItems;
            bool m_dirty;
            Placement m_placement;
            bool m_accurate;
            Appearance m_appearance;
            BusyMode m_busyMode;
    };

}

#endif
