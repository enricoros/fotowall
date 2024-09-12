/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FotowallFile.h"

#include "App.h"
#include "Canvas/Canvas.h"
#include "Settings.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QTextStream>

/** \brief XML File structure

  Old (0.4 to 0.8.96)
    <fotowall>
      <project>
        <mode />
      </project>
      <desk />
      <content>
        <type .. />
        ...
        <type .. />
      </content>
    </fotowall>

  Since Fotowall 0.8.97
    <fotowall format="2" version="0.8.97">
      <canvas>
        <mode />
        <background />
        <content />
      </canvas>
    </fotowall>

**/

bool FotowallFile::read(const QString & fwFilePath, Canvas * canvas, bool inHistory)
{
  // open the file for reading
  QFile file(fwFilePath);
  if(!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(0, QObject::tr("Loading error"),
                          QObject::tr("Unable to load the Fotowall file %1").arg(fwFilePath));
    return false;
  }

  // load the DOM
  QString error;
  QDomDocument doc;
  if(!doc.setContent(&file, false, &error))
  {
    QMessageBox::critical(
        0, QObject::tr("Parsing error"),
        QObject::tr("Unable to parse the Fotowall file %1. The error was: %2").arg(fwFilePath, error));
    return false;
  }
  file.close();

  // get the Canvas node
  QDomElement root = doc.documentElement();
  QDomElement canvasElement = root.firstChildElement("canvas");
  if(!canvasElement.isElement()) // 'Format 1'
    canvasElement = root;
  if(!canvasElement.isElement()) return false;

  // restore the canvas
  canvas->setFilePath(fwFilePath);
  canvas->loadFromXml(canvasElement);

  // add to the recent history
  if(inHistory) App::settings->addRecentFotowallUrl(QUrl(fwFilePath));
  return true;
}

QImage FotowallFile::embeddedPreview(const QString & fwFilePath)
{
  // open the file for reading
  QFile file(fwFilePath);
  if(!file.open(QIODevice::ReadOnly)) return QImage();

  // load the DOM
  QDomDocument doc;
  if(!doc.setContent(&file, false)) return QImage();
  file.close();

  // get the Canvas/Preview element
  QDomElement previewElement = doc.documentElement().firstChildElement("canvas").firstChildElement("preview");
  if(previewElement.isElement())
  {
    QString imageTextData = previewElement.text();
    QByteArray imageData = QByteArray::fromBase64(imageTextData.toLatin1());
    return QImage::fromData(imageData, "PNG");
  }

  return QImage();
}

bool FotowallFile::saveV2(const QString & fwFilePath, const Canvas * canvas)
{
  // create the document
  QDomDocument doc;
  doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" "));

  QDomElement rootElement = doc.createElement("fotowall");
  rootElement.setAttribute("format", 2);
  rootElement.setAttribute("version", QCoreApplication::applicationVersion());
  doc.appendChild(rootElement);

  QDomElement canvasElement = doc.createElement("canvas");
  rootElement.appendChild(canvasElement);

  // save current canvas
  Canvas * rwCanvas = (Canvas *)canvas;
  rwCanvas->setFilePath(fwFilePath);
  canvas->saveToXml(canvasElement);

  // open the file for writing
  QFile file(fwFilePath);
  if(!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::warning(0, QObject::tr("File Error"),
                         QObject::tr("Error saving to the Fotowall file '%1'").arg(fwFilePath));
    return false;
  }

  // save in the file (2 indent spaces)
  QTextStream outStream(&file);
  doc.save(outStream, 2);
  file.close();

  // store a reference to the just written file
  App::settings->addRecentFotowallUrl(QUrl(fwFilePath));
  return true;
}

QString FotowallFile::getLoadFotowallFile()
{
  // make up the default load path (stored as 'Fotowall/LoadProjectDir')
  QString defaultLoadPath = App::settings->value("Fotowall/LoadProjectDir").toString();

  // ask the 'load' file name
  QString fwFilePath =
      QFileDialog::getOpenFileName(0, QObject::tr("Select the Fotowall file"), defaultLoadPath,
                                   QObject::tr("Fotowall (*.fotowall)") /*, 0, QFileDialog::DontResolveSymlinks*/);
  if(fwFilePath.isNull()) return QString();

  // store the load path to settings
  App::settings->setValue("Fotowall/LoadProjectDir", QFileInfo(fwFilePath).absolutePath());
  return fwFilePath;
}

QStringList FotowallFile::getLoadFotowallFiles()
{
  // make up the default load path (stored as 'Fotowall/LoadProjectDir')
  QString defaultLoadPath = App::settings->value("Fotowall/LoadProjectDir").toString();

  // ask the 'load' file name
  QStringList fwFilePaths =
      QFileDialog::getOpenFileNames(0, QObject::tr("Add Fotowall files to the Canvas"), defaultLoadPath,
                                    QObject::tr("Fotowall (*.fotowall)") /*, 0, QFileDialog::DontResolveSymlinks*/);
  if(fwFilePaths.isEmpty()) return QStringList();

  // store the load path to settings
  App::settings->setValue("Fotowall/LoadProjectDir", QFileInfo(fwFilePaths[0]).absolutePath());
  return fwFilePaths;
}

QString FotowallFile::getSaveFotowallFile(const QString & defaultFilePath)
{
  // make up the default save path
  QString defaultSavePath = defaultFilePath;
  if(App::settings->contains("Fotowall/SaveProjectDir") && !defaultFilePath.contains(QDir::separator()))
    defaultSavePath.prepend(App::settings->value("Fotowall/SaveProjectDir").toString() + QDir::separator());

  // ask the 'save' file name
  QString fwFilePath = QFileDialog::getSaveFileName(0, QObject::tr("Select the Fotowall file"), defaultSavePath,
                                                    "Fotowall (*.fotowall)");
  if(fwFilePath.isNull()) return QString();

  // store the save path to settings
  App::settings->setValue("Fotowall/SaveProjectDir", QFileInfo(fwFilePath).absolutePath());

  // add extension, if missing
  if(!fwFilePath.endsWith(".fotowall", Qt::CaseInsensitive)) fwFilePath += ".fotowall";
  return fwFilePath;
}
