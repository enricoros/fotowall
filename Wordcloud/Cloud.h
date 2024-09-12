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

#ifndef __Cloud_h__
#define __Cloud_h__

#include "Bits.h"
#include "WordItem.h"
#include <QDomElement>
#include <QObject>

namespace Wordcloud
{

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

  void setPlacement(Wordcloud::Placement);
  Wordcloud::Placement placement() const;

  void setAccurate(bool accurate);
  bool accurate() const;

  void setAppearance(const Wordcloud::Appearance &);
  Wordcloud::Appearance appearance() const;

  void setBusyMode(Wordcloud::BusyMode mode);
  Wordcloud::BusyMode busyMode() const;

  // load/save
  bool loadFromXml(QDomElement & cloudElement);
  void saveToXml(QDomElement & cloudElement) const;

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

} // namespace Wordcloud

#endif
