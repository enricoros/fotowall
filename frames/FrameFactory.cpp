/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
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
#include <QFile>
#include <QSettings>

Q_GLOBAL_STATIC(FrameFactory, d)
#define FRAME_DEF 0x0001
#define FRAME_HEART 0x0002
#define SVG_BASE_CLASS 0x1000
#define FRAME_SVG_1 ":/plasma-frames/1.svg"
#define FRAME_SVG_2 ":/plasma-frames/2.svg"
#define DEFAULT_FRAME 0x1000


// STATICS
QList<quint32> FrameFactory::classes()
{
    QList<quint32> classes = d()->m_svgMap.keys();
    classes.prepend(FRAME_DEF);
    classes.append(FRAME_HEART);
    return classes;
}

Frame * FrameFactory::createFrame(quint32 frameClass)
{
    if (frameClass == FRAME_DEF)
        return new StandardFrame();
    else if (frameClass == FRAME_HEART)
        return new HeartFrame();
    else if (d()->m_svgMap.contains(frameClass))
        return new PlasmaFrame(frameClass, d()->m_svgMap[ frameClass ]);
    else
        qWarning( "FrameFactory::createFrame: unknown frame for class %x, falling back to default", frameClass);
    return 0;
}

Frame * FrameFactory::defaultFrame()
{
    return createFrame(d()->m_defaultClass);
}

void FrameFactory::addSvgFrame(const QString & fileName)
{
    d()->m_svgMap[ d()->m_svgClassIndex++ ] = fileName;
}

quint32 FrameFactory::defaultFrameClass()
{
    return d()->m_defaultClass;
}

void FrameFactory::setDefaultFrameClass(quint32 frameClass)
{
    d()->m_defaultClass = frameClass;
}

// class impl
FrameFactory::FrameFactory()
    : m_defaultClass(DEFAULT_FRAME)
{
    // init default frames
    m_svgClassIndex = SVG_BASE_CLASS;
    m_svgMap[m_svgClassIndex++] = FRAME_SVG_1;
    m_svgMap[m_svgClassIndex++] = FRAME_SVG_2;

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
