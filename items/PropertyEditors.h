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

#ifndef __PropertyEditors_h__
#define __PropertyEditors_h__

#include <QObject>
#include <QAbstractSlider>
#include <QMetaProperty>
#include <QPointer>

class PE_AbstractSlider : public QObject
{
    Q_OBJECT
    public:
        PE_AbstractSlider(QAbstractSlider * slider, QObject * target, const char * propertyName, QObject * parent = 0);
        ~PE_AbstractSlider();

        bool isValid() const;

    private:
        QPointer<QAbstractSlider> m_slider;
        QPointer<QObject> m_target;
        QMetaProperty m_property;
        bool m_isValid;

    private Q_SLOTS:
        void slotSliderValueChanged(int);
        void slotPropertyChanged();
};

#endif
