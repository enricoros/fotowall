/***************************************************************************
 *                                                                         *
 *   This file is part of the Wordcloud project,                           *
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

#ifndef __Scanner_h__
#define __Scanner_h__

#include <QObject>
#include <QLocale>
#include <QUrl>
#include "Cloud.h"
class QTableWidget;

namespace Wordcloud {

    class Scanner : public QObject
    {
        public:
            // inputs
            bool addFromFile(const QString & fileName);
            bool addFromString(const QString & string);
            bool addFromUrl(const QUrl & url);
            bool addFromRss(const QUrl & rss);
            void clear();

            // outputs
            WordList takeWords(bool cleanList);
            int wordCount() const;
            bool isEmpty() const;

            // debug
            void dumpOnTable(QTableWidget * table);
            void dumpWords() const;

        private:
            void addWord(const QString & word);
            void removeWordsByLanguage(QLocale::Language language);
            void removeWordsBelowCount(int count);
            WordList m_words;
    };

}

#endif
