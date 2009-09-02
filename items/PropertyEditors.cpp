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

#include "PropertyEditors.h"
#include <QDebug>

PE_AbstractSlider::PE_AbstractSlider(QAbstractSlider * _slider, QObject * _target, const char * propertyName, QObject * parent)
  : QObject(parent)
  , m_slider(_slider)
  , m_target(_target)
  , m_isValid(false)
{
    // find the property
    int idx = m_target->metaObject()->indexOfProperty(propertyName);
    if (idx == -1) {
        qWarning("PE_AbstractSlider: target has no property '%s'", propertyName ? propertyName : "NULL");
        return;
    }
    m_property = m_target->metaObject()->property(idx);

    // read initial value and link to property changes
    slotPropertyChanged();
#if QT_VERSION >= 0x040500
    if (m_property.hasNotifySignal()) {
        QMetaMethod notifySignal = m_property.notifySignal();
        int nameLength = qstrlen(notifySignal.signature());
        char signalName[nameLength + 2];
        signalName[0] = '0' + QSIGNAL_CODE;
        qstrcpy(signalName + 1, notifySignal.signature());
        connect(m_target.data(), signalName, this, SLOT(slotPropertyChanged()));
    }
#endif

    // link to the slider changes
    connect(m_slider.data(), SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

    // allow Int and Double properties only
    if (m_property.type() != QVariant::Int && m_property.type() != QVariant::Double)
        qWarning("PE_AbstractSlider: unhandled property '%s' of type %d", propertyName, (int)m_property.type());
    else
        m_isValid = true;
}

bool PE_AbstractSlider::isValid() const
{
    return m_isValid;
}

void PE_AbstractSlider::slotSliderValueChanged(int intValue)
{
    // skip if link dropped
    if (!m_slider || !m_target)
        return;

    // QVariant::Int: properties are directly linked to the slider
    if (m_property.type() == QVariant::Int) {
        m_property.write(m_target.data(), intValue);
    }

    // QVariant::Double: remap to the 0..1 range
    else if (m_property.type() == QVariant::Double) {
        if (m_slider->maximum() > m_slider->minimum()) {
            qreal realVal = (qreal)(intValue - m_slider->minimum()) / (qreal)(m_slider->maximum() - m_slider->minimum());
            m_property.write(m_target.data(), realVal);
        }
    }
}

void PE_AbstractSlider::slotPropertyChanged()
{
    // skip if link dropped
    if (!m_slider || !m_target)
        return;

    // QVariant::Int: slider gets the property value
    if (m_property.type() == QVariant::Int) {
        int intValue = m_property.read(m_target.data()).toInt();
        m_slider->setValue(intValue);
    }

    // QVariant::Double: slider is scrolled from start to stop for the 0..1 property value
    else if (m_property.type() == QVariant::Double) {
        qreal realValue = m_property.read(m_target.data()).toDouble();
        int intValue = m_slider->minimum() + (int)(realValue * (m_slider->maximum() - m_slider->minimum()));
        m_slider->setValue(intValue);
    }
}




PE_AbstractButton::PE_AbstractButton(QAbstractButton * _button, QObject * _target, const char * propertyName, QObject * parent)
  : QObject(parent)
  , m_button(_button)
  , m_target(_target)
  , m_isValid(false)
{
    // find the property
    int idx = m_target->metaObject()->indexOfProperty(propertyName);
    if (idx == -1) {
        qWarning("PE_AbstractButton: target has no property '%s'", propertyName ? propertyName : "NULL");
        return;
    }
    m_property = m_target->metaObject()->property(idx);

    // read initial value and link to property changes
    slotPropertyChanged();
#if QT_VERSION >= 0x040500
    if (m_property.hasNotifySignal()) {
        QMetaMethod notifySignal = m_property.notifySignal();
        int nameLength = qstrlen(notifySignal.signature());
        char signalName[nameLength + 2];
        signalName[0] = '0' + QSIGNAL_CODE;
        qstrcpy(signalName + 1, notifySignal.signature());
        connect(m_target.data(), signalName, this, SLOT(slotPropertyChanged()));
    }
#endif

    // link to the abstract button checkstate change
    connect(m_button.data(), SIGNAL(toggled(bool)), this, SLOT(slotButtonChecked(bool)));

    // allow Bool properties only
    if (m_property.type() != QVariant::Bool)
        qWarning("PE_AbstractSlider: unhandled property '%s' of type %d", propertyName, (int)m_property.type());
    else
        m_isValid = true;
}

bool PE_AbstractButton::isValid() const
{
    return m_isValid;
}

void PE_AbstractButton::slotButtonChecked(bool boolValue)
{
    // set the property to the current state of the button
    if (m_button && m_target && m_property.type() == QVariant::Int)
        m_property.write(m_target.data(), boolValue);
}

void PE_AbstractButton::slotPropertyChanged()
{
    // set the button check state as the bool property
    if (m_button && m_target && m_property.type() == QVariant::Bool) {
        bool boolValue = m_property.read(m_target.data()).toBool();
        m_button->setChecked(boolValue);
    }
}
