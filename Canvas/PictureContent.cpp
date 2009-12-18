/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "PictureContent.h"

#include "Frames/Frame.h"
#include "Shared/CPixmap.h"
#include "Shared/CroppingDialog.h"
#include "Shared/PanePropertyEditor.h"
#include "Shared/PropertyEditors.h"
#include "Shared/RenderOpts.h"
#include "ButtonItem.h"
#include "PictureProperties.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QProcess>
#include <QTimer>
#include <QUrl>

PictureContent::PictureContent(QGraphicsScene * scene, bool disableFading, QGraphicsItem * parent)
    : AbstractContent(scene, !disableFading, false, parent)
    , m_photo(0)
    , m_opaquePhoto(false)
    , m_progress(0)
    , m_netWidth(0)
    , m_netHeight(0)
    , m_netReply(0)
    , m_watcher(0)
    , m_watcherTimer(0)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("..."));

    // allow dropping
    setAcceptDrops(true);

    // add flipping buttons
    ButtonItem * bFlipH = new ButtonItem(ButtonItem::FlipH, Qt::blue, QIcon(":/data/action-flip-horizontal.png"), this);
    bFlipH->setToolTip(tr("Flip horizontally"));
    bFlipH->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    connect(bFlipH, SIGNAL(clicked()), this, SIGNAL(flipHorizontally()));
    addButtonItem(bFlipH);

    ButtonItem * bFlipV = new ButtonItem(ButtonItem::FlipV, Qt::blue, QIcon(":/data/action-flip-vertical.png"), this);
    bFlipV->setToolTip(tr("Flip vertically"));
    bFlipV->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    addButtonItem(bFlipV);
    connect(bFlipV, SIGNAL(clicked()), this, SIGNAL(flipVertically()));

#if 0
    // add cropping button (TODO: enable this?)
    ButtonItem * bCrop = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-scale.png"), this);
    bCrop->setToolTip(tr(""));
    bCrop->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    addButtonItem(bCrop);
    connect(bCrop, SIGNAL(clicked()), this, SIGNAL(requestCrop()));
#endif
}

PictureContent::~PictureContent()
{
    dropNetworkConnection();
    delete m_watcherTimer;
    delete m_watcher;
    delete m_photo;
}

bool PictureContent::loadPhoto(const QString & picFilePath, bool keepRatio, bool setName)
{
    dropNetworkConnection();
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = 0;
    m_fileUrl = QString();
    m_netWidth = 0;
    m_netHeight = 0;

    m_photo = new CPixmap(picFilePath);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        return false;
    }

    m_opaquePhoto = !m_photo->hasAlpha();
    m_fileUrl = QDir(picFilePath).canonicalPath();
    if (m_fileUrl.isEmpty())
        m_fileUrl = picFilePath;
    if (keepRatio)
        resetContentsRatio();
    if (setName) {
        QString string = QFileInfo(picFilePath).fileName().section('.', 0, 0);
        string = string.mid(0, 10);
        setFrameText(string + tr("..."));
    }
    applyPostLoadEffects();

    // notify image change
    emit contentChanged();
    return true;
}

bool PictureContent::loadFromNetwork(const QString & url, QNetworkReply * reply, const QString & title, int width, int height)
{
    dropNetworkConnection();
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = 0;
    m_fileUrl = url;
    m_netWidth = width;
    m_netHeight = height;

    // start a download if not passed as a paramenter
    if (!reply) {
        // the QNAM will be auto-deleted on closure
        QNetworkAccessManager * nam = new QNetworkAccessManager(this);
        QNetworkRequest request(url);
        m_netReply = nam->get(request);
    } else
        m_netReply = reply;

    // set title
    if (!title.isEmpty()) {
        setFrameTextEnabled(true);
        setFrameText(title);
    }

#if QT_VERSION >= 0x040600
    // Immediate Decode: just handle the reply if done
    if (m_netReply->isFinished())
        return slotLoadNetworkData();
#else
    // No Precaching ensures signals to be emitted later
#endif

    // Deferred Decode: listen to the network job
    setAcceptHoverEvents(false);
    setControlsVisible(false);
    m_progress = 0.01;
    connect(m_netReply, SIGNAL(finished()), this, SLOT(slotLoadNetworkData()));
    connect(m_netReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotNetworkError()));
    connect(m_netReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(slotNetworkProgress(qint64,qint64)));

    // reset size, if got the network one
    if (m_netWidth > 0 && m_netHeight > 0)
        resetContentsRatio();
    return true;
}

bool PictureContent::loadPixmap(const QPixmap & pixmap, const QString & title)
{
    dropNetworkConnection();
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = 0;
    m_fileUrl = QString();
    m_netWidth = 0;
    m_netHeight = 0;

    m_photo = new CPixmap(pixmap.toImage());
    m_opaquePhoto = !pixmap.hasAlpha();
    m_fileUrl = QString("data:/");
    resetContentsRatio();
    setFrameTextEnabled(!title.isEmpty());
    setFrameText(title);
    applyPostLoadEffects();

    // notify image change
    emit contentChanged();
    return true;
}

void PictureContent::addEffect(const PictureEffect & effect)
{
    if (!m_photo)
        return;

    m_photo->addEffect(effect);
    // adapt picture ratio after cropping
    if (effect.effect == PictureEffect::Crop) {
        QRect actualContentRect = contentRect();
        if ((actualContentRect.height() + actualContentRect.width()) > 0) {
            qreal reduceRatio = (qreal)(effect.rect.width()+effect.rect.height())/
                                (qreal)(actualContentRect.height() +actualContentRect.width());
            int newW = (int)((qreal)effect.rect.width()/reduceRatio);
            int newH = (int)((qreal)effect.rect.height()/reduceRatio);
            resizeContents(QRect(-newW/2, -newH/2, newW, newH), true);
        }
    }
#if QT_VERSION >= 0x040500
    else if(effect.effect == PictureEffect::Opacity)
        setOpacity(effect.param);
#endif
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED();

    // notify image change
    emit contentChanged();
}

void PictureContent::crop()
{
    if (!m_photo)
        return;
    CroppingDialog dial(m_photo);
    if (dial.exec() != QDialog::Accepted)
        return;
    QRect cropRect = dial.getCroppingRect();
    if (!cropRect.isNull())
        addEffect(PictureEffect(PictureEffect::Crop, 0, cropRect));
}

QWidget * PictureContent::createPropertyWidget(ContentProperties * __p)
{
    PictureProperties * pp = __p ? (PictureProperties *)__p : new PictureProperties;
    AbstractContent::createPropertyWidget(pp);

    // properties link
#ifdef Q_WS_WIN
    delete pp->gimpButton;
#else
    new PE_AbstractButton(pp->gimpButton, this, "externalEdit", pp);
#endif

    return pp;
}

bool PictureContent::fromXml(QDomElement & contentElement, const QDir & baseDir)
{
    AbstractContent::fromXml(contentElement, baseDir);

    // build the afterload effects list
    m_afterLoadEffects.clear();
    QDomElement effectsE = contentElement.firstChildElement("effects");
    for (QDomElement effectE = effectsE.firstChildElement("effect"); effectE.isElement(); effectE = effectE.nextSiblingElement("effect")) {
        PictureEffect fx;
        fx.effect = (PictureEffect::Effect)effectE.attribute("type").toInt();
        fx.param = effectE.attribute("param").toDouble();
        if (fx.effect == PictureEffect::Crop) {
            QString rect = effectE.attribute("croppingRect");
            QStringList coordinates = rect.split(" ");
            if(coordinates.size() >= 3) {
                QRect croppingRect (coordinates.at(0).toInt(), coordinates.at(1).toInt(), coordinates.at(2).toInt(), coordinates.at(3).toInt());
                fx.rect = croppingRect;
            }
        }
#if QT_VERSION >= 0x040500
        else if (fx.effect == PictureEffect::Opacity)
            setOpacity(fx.param);
#endif
        m_afterLoadEffects.append(fx);
    }

    // load picture properties
    QString path;

    // try relative file path
    const QString relPath = contentElement.firstChildElement("relativeFilePath").text();
    if (!relPath.isEmpty() && baseDir.exists(relPath))
        path = QDir::cleanPath(baseDir.filePath(relPath));

    // or use absolute path/url
    if (path.isEmpty())
        path = contentElement.firstChildElement("fileUrl").text();

    // RETROCOMP <= 0.8
    if (path.isEmpty())
        path = contentElement.firstChildElement("path").text();

    // load Network image
    if (path.startsWith("http", Qt::CaseInsensitive) || path.startsWith("ftp", Qt::CaseInsensitive))
        return loadFromNetwork(path, 0);

    // look for the file if can't find it anymore
    if (!QFile::exists(path)) {
        QString searchFileName = QFileInfo(path).fileName();
        if (!searchFileName.isEmpty()) {

            // find all replacements from the current basepath
            qWarning("PictureContent::fromXml: file '%s' not found, scanning for replacements", qPrintable(path));
            QDirIterator dIt(baseDir, QDirIterator::Subdirectories);
            QStringList replacements;
            while (dIt.hasNext()) {
                dIt.next();
                if (dIt.fileName() == searchFileName) {
                    QString replacement = dIt.fileInfo().absoluteFilePath();
                    replacements.append(replacement);
                    qWarning("PictureContent::fromXml:    found '%s'", qPrintable(replacement));
                }
            }

            // use the first replacement (### 1.0 display a selection dialog)
            if (!replacements.isEmpty()) {
                path = replacements.first();
                qWarning("PictureContent::fromXml:    using '%s'", qPrintable(path));
            } else
                qWarning("PictureContent::fromXml:    no replacements found");
        }
    }

    // load Local image
    return loadPhoto(path, false, false);
}

void PictureContent::toXml(QDomElement & contentElement, const QDir & baseDir) const
{
    // save AbstractContent properties and rename to 'picture'
    AbstractContent::toXml(contentElement, baseDir);
    contentElement.setTagName("picture");

    // save picture properties
    QDomDocument doc = contentElement.ownerDocument();
    QDomElement domElement;

    // save image url (whether is a local path or remote url)
    if (!m_fileUrl.isEmpty() && !m_fileUrl.startsWith("data:")) {

        // if file, save relative path (mandatory for relocating files)
        if (!m_fileUrl.startsWith("http:", Qt::CaseInsensitive) && !m_fileUrl.startsWith("ftp:", Qt::CaseInsensitive)) {
            QString relativePath = baseDir.relativeFilePath(m_fileUrl);
            if (!relativePath.isEmpty()) {
                domElement = doc.createElement("relativeFilePath");
                contentElement.appendChild(domElement);
                domElement.appendChild(doc.createTextNode(relativePath));
            }
        }

        // save the url (mostly useful for non-local files)
        domElement = doc.createElement("fileUrl");
        contentElement.appendChild(domElement);
        domElement.appendChild(doc.createTextNode(m_fileUrl));
    }

    // save the effects
    domElement = doc.createElement("effects");
    contentElement.appendChild(domElement);
    QList<PictureEffect> effectsList = m_afterLoadEffects;
    if (m_photo)
#if QT_VERSION >= 0x040500
        effectsList.append(m_photo->effects());
#else
        foreach(const PictureEffect & effect, m_photo->effects())
            effectsList.append(effect);
#endif
    foreach (const PictureEffect & effect, effectsList) {
        QDomElement effectElement = doc.createElement("effect");
        effectElement.setAttribute("type", effect.effect);
        effectElement.setAttribute("param", effect.param);
        if(effect.effect == PictureEffect::Crop) {
            QString croppingRectStr;
            croppingRectStr = QString::number(effect.rect.x()) + " " + QString::number(effect.rect.y())
                + " " + QString::number(effect.rect.width()) + " " + QString::number(effect.rect.height());

            effectElement.setAttribute("croppingRect", croppingRectStr );
        }
        domElement.appendChild(effectElement);
    }
}

void PictureContent::drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio)
{
    // draw progress
    if (m_progress > 0.0 && m_progress < 1.0 && !RenderOpts::HQRendering) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::blue);
        painter->drawPie(QRect(targetRect.center(), QSize(20, 20)), 90 * 16, (int)(-5760.0 * m_progress));
    }

    // skip if no photo
    if (!m_photo)
        return;

    // blit if opaque picture
#if QT_VERSION >= 0x040600
    //disabled for 4.6 too, since it relies on raster.
    //if (m_opaquePhoto)
    //    painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

    // draw high-resolution photo when exporting png
    if (RenderOpts::HQRendering || ratio != Qt::IgnoreAspectRatio) {
        QSize scaledSize = m_photo->size();
        scaledSize.scale(targetRect.size(), ratio);
        int offX = targetRect.left() + (targetRect.width() - scaledSize.width()) / 2;
        int offY = targetRect.top() + (targetRect.height() - scaledSize.height()) / 2;
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
        painter->drawPixmap(offX, offY, scaledSize.width(), scaledSize.height(), *m_photo);
        return;
    }

    // draw photo using caching and deferred rescales
    if (beingTransformed()) {
        if (!m_cachedPhoto.isNull())
            painter->drawPixmap(targetRect, m_cachedPhoto);
    } else {
        if (m_cachedPhoto.isNull() || m_cachedPhoto.size() != targetRect.size())
            m_cachedPhoto = m_photo->scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter->drawPixmap(targetRect.topLeft(), m_cachedPhoto);
    }

#if QT_VERSION >= 0x040600
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

QPixmap PictureContent::toPixmap(const QSize & size, Qt::AspectRatioMode ratio)
{
    if (m_photo)
        return ratioScaledPixmap(m_photo, size, ratio);
    return AbstractContent::toPixmap(size, ratio);
}

int PictureContent::contentHeightForWidth(int width) const
{
    if (m_netWidth > 0 && m_netHeight > 0)
        return (m_netHeight * width) / m_netWidth;
    if (!m_photo || m_photo->width() < 1)
        return -1;
    return (m_photo->height() * width) / m_photo->width();
}

bool PictureContent::contentOpaque() const
{
    return m_opaquePhoto;
}

void PictureContent::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // load the first valid picture
    foreach (const QUrl & url, event->mimeData()->urls()) {
        // handle network drops
        if (url.scheme() == "http" || url.scheme() == "ftp") {
            if (loadFromNetwork(url.toString(), 0)) {
                event->accept();
                return;
            }
        }
        // handle local drops
        QString picFilePath = url.toString();
        if (QFile::exists(picFilePath)) {
            if (loadPhoto(picFilePath, true, true)) {
                event->accept();
                return;
            }
        }
    }
}

void PictureContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit requestBackgrounding();
}

void PictureContent::setExternalEdit(bool enabled)
{
    if (!m_photo)
        return;

    // start gimp if requested
    if (enabled && !m_watcher) {
        // save the pic to a file
        QString tmpFile = QDir::tempPath() + QDir::separator() + "TEMP" + QString::number(qrand() % 999999) + ".png";
        if (!m_photo->save(tmpFile, "PNG")) {
            qWarning("PictureContent::slotGimpEdit: can't save the image");
            return;
        }

        // open it with the gimp
#ifdef Q_OS_WIN32
        QString executable = "gimp.exe";
#else
        QString executable = "gimp";
#endif
        if (!QProcess::startDetached(executable, QStringList() << tmpFile)) {
            qWarning("PictureContent::slotGimpEdit: can't start The Gimp");
            return;
        }

        // start a watcher over it
        delete m_watcher;
        m_watcher = new QFileSystemWatcher(this);
        m_watcher->setProperty("fullName", tmpFile);
        m_watcher->addPath(tmpFile);
        connect(m_watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(slotGimpCompressNotifies()));
        return;
    }

    // delete if requested
    if (m_watcher && !enabled) {
        delete m_watcherTimer;
        m_watcherTimer = 0;
        delete m_watcher;
        m_watcher = 0;
    }
}

bool PictureContent::externalEdit() const
{
    return m_watcher;
}

void PictureContent::dropNetworkConnection()
{
    if (m_netReply) {
        m_netReply->disconnect(0,0,0);
#if QT_VERSION >= 0x040600
        if (!m_netReply->isFinished())
#endif
            m_netReply->abort();
        m_netReply->deleteLater();
        m_netReply = 0;
    }
    m_progress = 0.0;
}

void PictureContent::applyPostLoadEffects()
{
    foreach (const PictureEffect & effect, m_afterLoadEffects)
        m_photo->addEffect(effect);
    m_afterLoadEffects.clear();
    update();
    GFX_CHANGED();
}

void PictureContent::slotGimpCompressNotifies()
{
    if (!m_watcherTimer) {
        m_watcherTimer = new QTimer(this);
        m_watcherTimer->setSingleShot(true);
        connect(m_watcherTimer, SIGNAL(timeout()), this, SLOT(slotGimpFinished()));
    }
    m_watcherTimer->start(500);
}

void PictureContent::slotGimpFinished()
{
    // get the file name and dispose the watcher
    if (!m_watcher)
        return;
    QString gimpFilePath = m_watcher->property("fullName").toString();

    // reload the file
    CPixmap * newPhoto = new CPixmap(gimpFilePath);
    if (newPhoto->isNull()) {
        qWarning("PictureContent::slotGimpFinished: can't load the modified picture");
        delete newPhoto;
        return;
    }
    delete m_photo;
    m_photo = newPhoto;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = !m_photo->hasAlpha();
    m_fileUrl = QString();
    update();

    // notify image change
    emit contentChanged();
}

bool PictureContent::slotLoadNetworkData()
{
    // get the data
    if (!m_netReply)
        return false;
    QByteArray replyData = m_netReply->readAll();
    dropNetworkConnection();

    // make the QImage from data
    QImage image = QImage::fromData(replyData);
    if (image.isNull())
        return false;

    // update contents
    m_netWidth = image.width();
    m_netHeight = image.height();
    resetContentsRatio();
    m_progress = 1.0;
    m_photo = new CPixmap(image);
    m_opaquePhoto = !m_photo->hasAlpha();
    applyPostLoadEffects();
    setAcceptHoverEvents(true);

    // notify image change
    emit contentChanged();
    return true;
}

void PictureContent::slotNetworkError()
{
    // clear state
    if (!m_netReply)
        return;
    dropNetworkConnection();
    m_progress = 0.0;
    setAcceptHoverEvents(true);
}

void PictureContent::slotNetworkProgress(qint64 a, qint64 b)
{
    m_progress = b > 0 ? (double)a / (double)b : 0.0;
    update();
}
