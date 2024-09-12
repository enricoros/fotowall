/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __VersionCheckDialog_h__
#define __VersionCheckDialog_h__

#include "Shared/MetaXmlReader.h"
#include <QDialog>
namespace Ui
{
class VersionCheckDialog;
}

class VersionCheckDialog : public QDialog
{
  Q_OBJECT
public:
  VersionCheckDialog(QWidget * parent = 0);
  ~VersionCheckDialog();

private Q_SLOTS:
  void slotFetched();
  void slotError(const QString & error);
  void slotDownload();

private:
  Ui::VersionCheckDialog * ui;
  MetaXml::Connector * m_connector;
  MetaXml::Release m_release;
};

#endif
