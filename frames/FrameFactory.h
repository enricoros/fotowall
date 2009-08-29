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

#ifndef __FrameFactory_h__
#define __FrameFactory_h__

#include <QMap>
#include <QString>
#include "Frame.h"

class FrameFactory
{
    public:
        // enumerate all available frames
        static QList<quint32> classes();

        // create a frame
        static Frame * createFrame(quint32 frameClass);
        static Frame * defaultPanelFrame();
        static Frame * defaultPictureFrame();

        // add new types
        static void addSvgFrame(const QString & fileName);

        // change the default class
        static quint32 defaultPanelClass();
        static void setDefaultPanelClass(quint32 frameClass);
        static quint32 defaultPictureClass();
        static void setDefaultPictureClass(quint32 frameClass);

    private:
        quint32 m_defaultPanel;
        quint32 m_defaultPicture;
        quint32 m_svgClassIndex;
        QMap<quint32, QString> m_svgMap;

    public:
        FrameFactory();
        ~FrameFactory();
};

#endif
