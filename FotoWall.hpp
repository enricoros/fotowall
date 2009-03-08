/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *   	   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>    *          
 *   Modified by Tanguy Arnaud <phparnsk8@gmail.com>, see CHANGLOG to have *
 *   summary of the modification.									       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FotoWall_h__
#define __FotoWall_h__

#include <QWidget>
#include <QLabel>
#include "ui_FotoWall.h"
#include <QGraphicsView>
class FWScene;

class FotoWall : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
	public:
		FotoWall(QMainWindow * parent = 0);
		~FotoWall();

	private:
		QGraphicsView * m_view;
		FWScene *       m_scene;


		private Q_SLOTS:
			void on_loadButton_clicked(); 
		void on_addPictures_clicked();  // Dialog to load Pictures
		void on_pictureBorderCheckbox_clicked();  // Show/Hide Rect around the picture
		void on_saveButton_clicked();
		void on_pngButton_clicked();
		void on_quitButton_clicked();
		void on_titleEdit_textChanged( const QString & text );
		void on_pictureBackgroundButton_clicked();
		//effects menu
		void on_actionBlack_and_White_triggered();
		void on_actionNVG_triggered();
		void on_actionLuminosity_triggered();
		void on_actionInvert_colors_triggered();
		void on_actionHorizontal_flip_triggered();
		void on_actionVertical_flip_triggered();

		//image menu
		void on_actionRotate_90_left_triggered();
		void on_actionRotate_90_right_triggered();
		void on_actionSet_as_background_triggered();

		//help menu
		void on_actionDocumentation_triggered();
};

#endif
