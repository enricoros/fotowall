/****************************
* Auteur :Tanguy Arnaud
* Date : lundi 7 juillet 2008, 10:09:21 (UTC+0200)
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

#ifndef ARNAUD_H_CPIXMAP
#define ARNAUD_H_CPIXMAP

#include <QPixmap>

class CPixmap : public QPixmap {
public:
	CPixmap(const QString &fileName);
	CPixmap(const QPixmap &pixmap);

	void toNVG();
	void invertColors(); 
	void flipH();
	void flipV();
	void toBlackAndWhite();
	void luminosity(int value);
	
	//accessors
	bool isNVG() { return m_isNVG; }
	bool isBlackAndWhite() { return m_isBlackAndWhite; }

	void setNVG(bool state=false) { m_isNVG = state; }
	void setBlackAndWhite(bool state=false) { m_isBlackAndWhite = state; }

private:
	QString m_filePath;
	bool m_isNVG;
	bool m_isBlackAndWhite;
};

#endif /* ARNAUD_H_CPIXMAP */
