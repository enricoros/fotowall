/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FrameFactory.h"
#include "HeartFrame.h"
#include "PlasmaFrame.h"
#include "StandardFrame.h"
#include "EmptyFrame.h"
#include <QFile>
#include <QSettings>

Q_GLOBAL_STATIC(FrameFactory, d)
#define FRAME_DEF 0x0001
#define FRAME_HEART 0x0002
#define FRAME_NOFRAME 0x0003
#define SVG_BASE_CLASS 0x1000
#define FRAME_SVG_1 ":/plasma-frames/1.svg"
#define FRAME_SVG_2 ":/plasma-frames/2.svg"
#define FRAME_SVG_3 ":/plasma-frames/3.svg"
#define FRAME_SVG_4 ":/plasma-frames/4.svg"
#define DEFAULT_PANEL_FRAME SVG_BASE_CLASS
#define DEFAULT_PICTURE_FRAME FRAME_DEF


// STATICS
QList<quint32> FrameFactory::classes()
{
    QList<quint32> classes = d()->m_svgMap.keys();
    classes.prepend(FRAME_DEF);
    classes.append(FRAME_HEART);
    classes.append(FRAME_NOFRAME);
    return classes;
}

Frame * FrameFactory::createFrame(quint32 frameClass)
{
    if (frameClass == Frame::NoFrame)
        return 0;
    else if (frameClass == FRAME_DEF)
        return new StandardFrame();
    else if (frameClass == FRAME_HEART)
        return new HeartFrame();
    else if (frameClass == FRAME_NOFRAME)
        return new EmptyFrame();
    else if (d()->m_svgMap.contains(frameClass))
        return new PlasmaFrame(frameClass, d()->m_svgMap[ frameClass ]);
    else
        qWarning( "FrameFactory::createFrame: unknown frame for class %x, falling back to default", frameClass);
    return 0;
}

Frame * FrameFactory::defaultPanelFrame()
{
    return createFrame(d()->m_defaultPanel);
}

Frame * FrameFactory::defaultPictureFrame()
{
    return createFrame(d()->m_defaultPicture);
}

void FrameFactory::addSvgFrame(const QString & fileName)
{
    d()->m_svgMap[ d()->m_svgClassIndex++ ] = fileName;
}

quint32 FrameFactory::defaultPanelClass()
{
    return d()->m_defaultPanel;
}

void FrameFactory::setDefaultPanelClass(quint32 frameClass)
{
    d()->m_defaultPanel = frameClass;
}

quint32 FrameFactory::defaultPictureClass()
{
    return d()->m_defaultPicture;
}

void FrameFactory::setDefaultPictureClass(quint32 frameClass)
{
    d()->m_defaultPicture = frameClass;
}

// class impl
FrameFactory::FrameFactory()
    : m_defaultPanel(DEFAULT_PANEL_FRAME)
    , m_defaultPicture(DEFAULT_PICTURE_FRAME)
{
    // init default frames
    m_svgClassIndex = SVG_BASE_CLASS;
    m_svgMap[m_svgClassIndex++] = FRAME_SVG_1;
    m_svgMap[m_svgClassIndex++] = FRAME_SVG_2;
    m_svgMap[m_svgClassIndex++] = FRAME_SVG_3;
    m_svgMap[m_svgClassIndex++] = FRAME_SVG_4;

    // add stored frames
    QSettings s;
    int values = s.value("frames/count", 0).toInt();
    for (int i = 0; i < values; i++) {
        QString fileName = s.value(QString("frames/frame%1").arg(i)).toString();
        if (QFile::exists(fileName))
            m_svgMap[m_svgClassIndex++] = fileName;
    }
}

FrameFactory::~FrameFactory()
{
    QSettings s;

    // store each filename
    int count = 0;
    QMap<quint32, QString>::iterator it = m_svgMap.begin(), end = m_svgMap.end();
    for (; it != end; ++it) {
        QString fileName = *it;
        if (fileName.startsWith(":/"))
            continue;
        s.setValue(QString("frames/frame%1").arg(count++), fileName);
    }
    // store the count
    if (count)
        s.setValue("frames/count", count);
}
