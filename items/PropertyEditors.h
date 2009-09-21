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
#include <QAbstractButton>
#include <QAbstractSlider>
#include <QMetaProperty>
#include <QPointer>
#include <QVariant>

template<class C>
class PE_TypeControl : public QObject
{
    public:
        PE_TypeControl(C * control, QObject * target, const char * propertyName, QObject * parent = 0)
          : QObject(parent)
          , m_control(control)
          , m_target(target)
          , m_isValid(false)
        {
            // find the property
            int idx = m_target->metaObject()->indexOfProperty(propertyName);
            if (idx == -1) {
                qWarning("PE_TypeControl: target has no property '%s'", propertyName ? propertyName : "NULL");
                return;
            }
            m_property = m_target->metaObject()->property(idx);
        }

        bool isValid() const
        {
            return m_isValid;
        }

    protected:
        QPointer<C> m_control;
        QPointer<QObject> m_target;
        QMetaProperty m_property;
        bool m_isValid;

    private:
        PE_TypeControl();
};

class PE_AbstractSlider : public PE_TypeControl<QAbstractSlider>
{
    Q_OBJECT
    public:
        PE_AbstractSlider(QAbstractSlider * slider, QObject * target, const char * propertyName, QObject * parent = 0);

    private Q_SLOTS:
        void slotSliderValueChanged(int);
        void slotPropertyChanged();
};

class PE_AbstractButton : public PE_TypeControl<QAbstractButton>
{
    Q_OBJECT
    public:
        PE_AbstractButton(QAbstractButton * button, QObject * target, const char * propertyName, QObject * parent = 0);

    private Q_SLOTS:
        void slotButtonChecked(bool checked);
        void slotPropertyChanged();
};

#endif
