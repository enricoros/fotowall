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

#ifndef __OcsTypes_h__
#define __OcsTypes_h__

#include <QObject>
#include <QList>

class KnowledgeItemV1 : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString status READ status WRITE setStatus)
    Q_PROPERTY(QString contentid READ contentid WRITE setContentid)
    Q_PROPERTY(QString user READ user WRITE setUser)
    Q_PROPERTY(QString changed READ changed WRITE setChanged)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString answer READ answer WRITE setAnswer)
    Q_PROPERTY(int comments READ comments WRITE setComments)
    Q_PROPERTY(QString detailpage READ detailpage WRITE setDetailpage)
    public:
        QString id() const {return m_id;}
        void setId(const QString & id) {m_id = id;}

        QString status() const {return m_status;}
        void setStatus(const QString & status) {m_status = status;}

        QString contentid() const {return m_contentid;}
        void setContentid(const QString & contentid) {m_contentid = contentid;}

        QString user() const {return m_user;}
        void setUser(const QString & user) {m_user = user;}

        QString changed() const {return m_changed;}
        void setChanged(const QString & changed) {m_changed = changed;}

        QString name() const {return m_name;}
        void setName(const QString & name) {m_name = name;}

        QString description() const {return m_description;}
        void setDescription(const QString & description) {m_description = description;}

        QString answer() const {return m_answer;}
        void setAnswer(const QString & answer) {m_answer = answer;}

        int comments() const {return m_comments;}
        void setComments(int comments) {m_comments = comments;}

        QString detailpage() const {return m_detailpage;}
        void setDetailpage(const QString & detailpage) {m_detailpage = detailpage;}

    private:
        QString m_id, m_status, m_contentid, m_user, m_changed, m_name, m_description, m_answer, m_detailpage;
        int m_comments;
};

typedef QList<KnowledgeItemV1 *> KnowledgeItemV1List;

#endif
