/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
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

// static global variable
bool globalExportingFlag = false;

class FWGraphicsView : public QGraphicsView {
    public:
        FWGraphicsView(FWScene * scene, QWidget * parent)
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


FotoWall::FotoWall(QWidget * parent)
    : QWidget(parent)
    , m_view(0)
    , m_scene(0)
{
    setupUi(this);
    setWindowIcon( QIcon(":/data/icon.png") );

    // create our custom scene
    m_scene = new FWScene(this);

    // add the graphicsview
    m_view = new FWGraphicsView(m_scene, centralWidget);
    QVBoxLayout * lay = new QVBoxLayout(centralWidget);
    lay->setSpacing(0);
    lay->setMargin(0);
    lay->addWidget(m_view);
}

FotoWall::~FotoWall()
{
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


void FotoWall::on_loadButton_clicked()
{
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

void FotoWall::on_saveButton_clicked()
{
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

#include <QDialog>
#include <QSpinBox>
class SizeDialog : public QDialog {
    public:
        SizeDialog(QWidget * parent = 0)
            : QDialog(parent)
        {
            setWindowTitle(tr("Select Resolution"));

            wSpin = new QSpinBox(this);
            wSpin->setRange(100, 10000);
            hSpin = new QSpinBox(this);
            hSpin->setRange(100, 10000);
            QPushButton * closeButton = new QPushButton(tr("OK"), this);
            connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

            QHBoxLayout * lay = new QHBoxLayout(this);
            lay->addWidget(wSpin);
            lay->addWidget(hSpin);
            lay->addWidget(closeButton);
        }
        QSpinBox * wSpin;
        QSpinBox * hSpin;
};

void FotoWall::on_pngButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Name a PNG file"), QDir::current().path(), "PNG Image (*.png)");
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
    int size = QFile(fileName).size();
    QMessageBox::information(this, tr("Image rendered"), tr("The target image is %1 bytes long").arg(size));
}

void FotoWall::on_quitButton_clicked()
{
    QCoreApplication::quit();
}

void FotoWall::on_titleEdit_textChanged(const QString & text)
{
    m_scene->setTitleText(text);
}

