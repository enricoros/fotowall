/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Modified by Tanguy Arnaud <phparnsk8@gmail.com>, see CHANGLOG to have *
 *   summary of the modification.										   *
 *   																	   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FotoWall.h"
#include "FWScene.h"
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QDebug>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include "ui_Aide.h"
#include "SizeDialog.h"
	#include "BackgroundChooser.h"
// static global variable
bool globalExportingFlag = false;
bool drawPictureBorder = true;
class FWGraphicsView : public QGraphicsView {
    public:
        FWGraphicsView(FWScene * scene, QWidget* parent)
            : QGraphicsView(scene, parent)
            , m_scene(scene)
        {
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setInteractive(true);
            setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::SmoothPixmapTransform */);
            setDragMode(QGraphicsView::NoDrag);
            setAcceptDrops(true);
            setFrameStyle(QFrame::NoFrame);
        }

    protected:
        void resizeEvent(QResizeEvent * /*event*/) {
            m_scene->resize(contentsRect().size());
        }

    private:
        FWScene * m_scene;
};


FotoWall::FotoWall(QMainWindow* parent)
    : QMainWindow(parent)
    , m_view(0)
    , m_scene(0)
{
	setupUi(this);
	setWindowIcon( QIcon(":/data/icon.png") );
	setWindowTitle ("Fotowall 0.23");


	// create our custom scene
	m_scene = new FWScene(this);

	QWidget* w = new QWidget; //The main widget in which the layouts are

	// add the graphicsview
	m_view = new FWGraphicsView(m_scene, Widget);

	QVBoxLayout *vLay = new QVBoxLayout; 
	vLay->setSpacing(0);
	vLay->setMargin(0);
	vLay->addWidget(cmdFrame);
	vLay->addWidget(Widget);
	w->setLayout(vLay);

	QVBoxLayout * lay = new QVBoxLayout(Widget);
	lay->setSpacing(0);
	lay->setMargin(0);
	lay->addWidget(m_view);

	setCentralWidget(w);
	connect(actionLoad, SIGNAL(triggered()), this, SLOT(on_loadButton_clicked()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(on_saveButton_clicked()));
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(on_quitButton_clicked()));
}
FotoWall::~FotoWall() {
    // dump current layout
    QFile file("autosave.lay");
    !file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    m_scene->save(out);
    file.close();

    // delete everything
    delete m_view;
    delete m_scene;
}

void FotoWall::on_loadButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Layout file"), QDir::current().path(), "Layouts (*.lay)");
    if (fileName.isNull())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Error"), tr("Error load layout from this file"));
        return;
    }
    QDataStream in(&file);
    m_scene->restore(in);
    titleEdit->setText(m_scene->titleText());
}
void FotoWall::on_addPictures_clicked() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this , tr("Select Picture File(s)"), QDir::current().path(), "Pictures (*.png *.jpg *.bmp)");
    if (fileNames.isEmpty())
        return;

		//Load all the selected pictures
		m_scene->loadPictures(&fileNames);
	
}
void FotoWall::on_pictureBorderCheckbox_clicked() {
	if (pictureBorderCheckbox->isChecked()) 
		drawPictureBorder = true;
	else
		drawPictureBorder = false;
	m_scene->FWupdate();
}
void FotoWall::on_saveButton_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select Layout file"), QDir::current().path(), "Layouts (*.lay)");
    if (fileName.isNull())
        return;
    if (!fileName.endsWith(".lay", Qt::CaseInsensitive))
        fileName += ".lay";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Save Error"), tr("Error saving layout to this file"));
        return;
    }
    QDataStream out(&file);
    m_scene->save(out);
}



void FotoWall::on_pngButton_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Name of PNG file"), QDir::current().path(), "PNG Image (*.png)");
    if (fileName.isNull())
        return;
    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
        fileName += ".png";

    // get the rendering size
    SizeDialog * sd = new SizeDialog(this);
    sd->wSpin->setValue(m_scene->width());
    sd->hSpin->setValue(m_scene->height());
    if (!sd->exec())
        return;
    int destW = sd->wSpin->value();
    int destH = sd->hSpin->value();
    delete sd;

    // render on the image
    globalExportingFlag = true;
    QImage image(destW, destH, QImage::Format_ARGB32);
    image.fill(0);
    QPainter imagePainter(&image);
    imagePainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    m_scene->render(&imagePainter, image.rect(), m_scene->sceneRect(), Qt::KeepAspectRatio);
    imagePainter.end();
    globalExportingFlag = false;

    // save image
    image.save(fileName, "PNG");

    if (!QFile::exists(fileName)) {
        QMessageBox::warning(this, tr("Rendering Save Error"), tr("Error rendering to this file"));
        return;
    }
    float size = QFile(fileName).size();
    QMessageBox::information(this, tr("Image rendered"), tr("The target image is %1 bytes long").arg(size));
}
void FotoWall::on_quitButton_clicked() {
	qApp->quit();
}
void FotoWall::on_titleEdit_textChanged(const QString & text) {
    m_scene->setTitleText(text);
}
void FotoWall::on_pictureBackgroundButton_clicked() {
QDialog *parent = new QDialog(this);
BackgroundChooser *backgroundChooser = new BackgroundChooser("data/backgrounds/", parent);
connect(backgroundChooser, SIGNAL(signalImageChoosed(QString)), m_scene, SLOT(slotCallChangeImageBackground(QString)));
parent->exec();
}

void FotoWall::on_actionBlack_and_White_triggered() {
	m_scene->callSlotToBlackAndWhite();
}
void FotoWall::on_actionNVG_triggered() {
	m_scene->callSlotToNVG();
}

void FotoWall::on_actionLuminosity_triggered() {
	m_scene->callSlotLuminosity();
}
void FotoWall::on_actionInvert_colors_triggered() {
	m_scene->callSlotInvertColors();
}
void FotoWall::on_actionHorizontal_flip_triggered() {
	m_scene->callSlotFlipH();
}
void FotoWall::on_actionVertical_flip_triggered() {
	m_scene->callSlotFlipV();
}
void FotoWall::on_actionRotate_90_left_triggered() {
	m_scene->callSlotRotate(-90);
}
void FotoWall::on_actionRotate_90_right_triggered() {
	m_scene->callSlotRotate(90);
}
void FotoWall::on_actionSet_as_background_triggered() {
	m_scene->setCurentImageAsBackground();
}

void FotoWall::on_actionDocumentation_triggered() {
	QDialog *fenetre = new QDialog;
    Ui::HelpDialog ui;
    ui.setupUi(fenetre);
	fenetre->show();
}
