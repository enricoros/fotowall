/****************************
* Auteur : Tanguy Arnaud
* Date : mardi 1 juillet 2008, 17:13:53 (UTC+0200)
* Version : 1.0
*
* Copyright (C) 2008 TANGUY Arnaud
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************/

#include <QtGui>
#include "ui_BackgroundChooser.h"

class BackgroundChooser : public QDialog,  public Ui_BackgroundChooser
{
	Q_OBJECT
	private:
		QDir *m_Dir;
		QString imageChoosed();
	public:
		BackgroundChooser(QString path, QDialog *parent=0);
		~BackgroundChooser();
	signals:
		void signalImageChoosed(QString);

	public Q_SLOTS:
		void slotOkButtonClicked();
		void slotAddOtherImage();
};
