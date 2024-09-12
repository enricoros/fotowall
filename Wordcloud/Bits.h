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

#ifndef __Bits_h__
#define __Bits_h__

#include <QColor>
#include <QFont>
#include <QList>
#include <QMap>
#include <QString>

namespace Wordcloud
{

/// Word statistical description
struct Word
{
  QString commonString;
  QString lowerString;
  int count;
  QMap<QString, int> variants;
};

// convenience alias
typedef QList<Word> WordList;

/// Looks of displayed data
struct Appearance
{
  // colors
  QList<QColor> textColors;
  QColor backColor;
  bool textGradient;
  // fonts
  QFont font;
  double minFontSize;
  double maxFontSize;
  bool quadraticFont;

  Appearance();
};

}; // namespace Wordcloud

#endif
