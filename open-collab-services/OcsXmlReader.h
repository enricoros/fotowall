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

#ifndef __OcsXmlReader_h__
#define __OcsXmlReader_h__

#include <QXmlStreamReader>
#include <QMap>
#include "OcsTypes.h"

class OcsXmlReader : public QXmlStreamReader
{
    public:
        OcsXmlReader(const QByteArray & data);
        ~OcsXmlReader();

        enum Flavour { KnowledgebaseList, KnowledgebaseGet };

        virtual bool read(Flavour flavour);

        // return the data structures
        QList<KnowledgeItemV1 *> takeKnowledgeItems();

    private:
        OcsXmlReader();

        void readEUnknown( bool verbose = true );
        void elementToObject(QObject * object);
        QMap<QString, QString> elementToMap();

        void readEMeta();
        void readEData();
        void readEKnowledgeItemV1();

        Flavour m_flavour;
        QList<KnowledgeItemV1 *> m_knowledgeItems;
};

#endif // OCSXMLREADER_H
