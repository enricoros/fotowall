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

#include "BackgroundChooser.h"

BackgroundChooser::BackgroundChooser(QString path, QDialog *parent):QDialog(parent) 
{
	setupUi(parent);
	listWidget->setViewMode(QListView::IconMode);
	m_Dir = new QDir(path);
	QStringList filters("*.png");
	QStringList pictureNames = m_Dir->entryList(filters);
	QStringList::iterator it;
	for(it = pictureNames.begin(); it!=pictureNames.end() ; it++) {
		QString path = m_Dir->absolutePath()+"/"+(*it);
		QListWidgetItem *item = new QListWidgetItem(QIcon(path), *it);
		item->setToolTip(path);
		listWidget->addItem(item);
	}
	 connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOkButtonClicked()));
	 connect(imageButton, SIGNAL(clicked()), this, SLOT(slotAddOtherImage()));
}

BackgroundChooser::~BackgroundChooser() {
	delete m_Dir;
}

QString BackgroundChooser::imageChoosed() {
	QList<QListWidgetItem *> selectedItems = listWidget->selectedItems();
	if(!selectedItems.isEmpty()){ 
		foreach(QListWidgetItem *item, selectedItems) {
			emit signalImageChoosed( item->toolTip() );
		}
	}
	return "";
}

void BackgroundChooser::slotOkButtonClicked() {
	imageChoosed();	
}

void BackgroundChooser::slotAddOtherImage() {
	QString imgPath = QFileDialog::getOpenFileName(this, "Open image", QString(), "Images (*.png *.gif *.jpg *.jpeg)");
	QFileInfo info(imgPath);
	QListWidgetItem *item = new QListWidgetItem(QIcon(imgPath), info.baseName());
	item->setToolTip(imgPath);
	listWidget->addItem(item);
}

