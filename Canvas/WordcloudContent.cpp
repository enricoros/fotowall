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

#include "WordcloudContent.h"

#include "Wordcloud/Scanner.h"

#include <QGraphicsScene>
#include <QFileDialog>
#include <QPainter>

WordcloudContent::WordcloudContent(QGraphicsScene * scene, QGraphicsItem * parent)
  : AbstractContent(scene, parent, false)
  , m_cloudScene(new QGraphicsScene)
  , m_cloud(new Wordcloud::Cloud)
  , m_cloudTaken(false)
{
    connect(m_cloudScene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintScene(const QList<QRectF> &)));
    m_cloud->setScene(m_cloudScene);
}

WordcloudContent::~WordcloudContent()
{
    if (m_cloudTaken)
        qWarning("WordcloudContent::~WordcloudContent: Wordcloud still exported");
    // this deletes even all the items of the cloud
    delete m_cloudScene;
    // this deletes only the container (BAD DONE, IMPROVE)
    delete m_cloud;
}

void WordcloudContent::manualInitialization()
{
    // temporarily get words
    Wordcloud::Scanner scanner;
    QString txtFilePath = QFileDialog::getOpenFileName(0, tr("Create a Wordcloud from a text file"));
    if (txtFilePath.isEmpty()) {
        scanner.addFromString(tr("Welcome to Wordcloud. Change options on the sidebar."));
        Wordcloud::WordList list = scanner.takeWords(false);
        Wordcloud::WordList::iterator wIt = list.begin();
        int ccc = list.size() + 1;
        while (wIt != list.end()) {
            wIt->count = ccc--;
            ++wIt;
        }
        m_cloud->newCloud(list);
    } else {
        scanner.addFromFile(txtFilePath);
        m_cloud->newCloud(scanner.takeWords(true));
    }
}

bool WordcloudContent::fromXml(QDomElement & contentElement, const QDir & baseDir)
{
    AbstractContent::fromXml(contentElement, baseDir);

    // restore the cloud
    QDomElement cloudElement = contentElement.firstChildElement("cloud");
    return m_cloud->loadFromXml(cloudElement);
}

void WordcloudContent::toXml(QDomElement & contentElement, const QDir & baseDir) const
{
    AbstractContent::toXml(contentElement, baseDir);
    contentElement.setTagName("wordcloud");

    // sanity check
    if (m_cloudTaken) {
        qWarning("WordcloudContent::toXml: resource taken, can't save it");
        return;
    }

    // dump cloud
    QDomDocument doc = contentElement.ownerDocument();
    QDomElement cloudElement = doc.createElement("cloud");
    contentElement.appendChild(cloudElement);
    m_cloud->saveToXml(cloudElement);
}

void WordcloudContent::drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio)
{
    Q_UNUSED(ratio)
    if (m_cloud)
        m_cloudScene->render(painter, targetRect, m_cloudScene->sceneRect(), Qt::KeepAspectRatio);
}

QVariant WordcloudContent::takeResource()
{
    // sanity check
    if (m_cloudTaken) {
        qWarning("WordcloudContent::takeResource: resource already taken");
        return QVariant();
    }

    // discard reference
    m_cloudTaken = true;
    Wordcloud::Cloud * cloud = m_cloud;
    m_cloud = 0;
    return qVariantFromValue((void *)cloud);
}

void WordcloudContent::returnResource(const QVariant & resource)
{
    // sanity checks
    if (!m_cloudTaken)
        qWarning("WordcloudContent::returnResource: not taken");
    if (m_cloud) {
        qWarning("WordcloudContent::returnResource: we already have a cloud, shouldn't return one");
        delete m_cloud;
    }

    // store reference back
    m_cloudTaken = false;
    m_cloud = static_cast<Wordcloud::Cloud *>(qVariantValue<void *>(resource));
    if (m_cloud)
        m_cloud->setScene(m_cloudScene);
    update();
}

void WordcloudContent::slotRepaintScene(const QList<QRectF> & /*exposed*/)
{
    update();
}
