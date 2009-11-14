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

#include "PropertyEditors.h"


PE_AbstractSlider::PE_AbstractSlider(QAbstractSlider * _slider, QObject * _target, const char * propertyName, QObject * parent)
  : PE_TypeControl<QAbstractSlider>(_slider, _target, propertyName, parent)
{
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
    connect(m_control.data(), SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

    // allow Int and Double properties only
    if (m_property.type() != QVariant::Int && m_property.type() != QVariant::Double)
        qWarning("PE_AbstractSlider: unhandled property '%s' of type %d", propertyName, (int)m_property.type());
    else
        m_isValid = true;
}

void PE_AbstractSlider::slotSliderValueChanged(int intValue)
{
    // skip if link dropped
    if (!m_control || !m_target)
        return;

    // QVariant::Int: properties are directly linked to the slider
    if (m_property.type() == QVariant::Int) {
        m_property.write(m_target.data(), intValue);
    }

    // QVariant::Double: remap to the 0..1 range
    else if (m_property.type() == QVariant::Double) {
        if (m_control->maximum() > m_control->minimum()) {
            qreal realVal = (qreal)(intValue - m_control->minimum()) / (qreal)(m_control->maximum() - m_control->minimum());
            m_property.write(m_target.data(), realVal);
        }
    }
}

void PE_AbstractSlider::slotPropertyChanged()
{
    // skip if link dropped
    if (!m_control || !m_target)
        return;

    // QVariant::Int: slider gets the property value
    if (m_property.type() == QVariant::Int) {
        int intValue = m_property.read(m_target.data()).toInt();
        m_control->setValue(intValue);
    }

    // QVariant::Double: slider is scrolled from start to stop for the 0..1 property value
    else if (m_property.type() == QVariant::Double) {
        qreal realValue = m_property.read(m_target.data()).toDouble();
        int intValue = m_control->minimum() + (int)(realValue * (m_control->maximum() - m_control->minimum()));
        m_control->setValue(intValue);
    }
}




PE_AbstractButton::PE_AbstractButton(QAbstractButton * _button, QObject * _target, const char * propertyName, QObject * parent)
  : PE_TypeControl<QAbstractButton>(_button, _target, propertyName, parent)
{
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
    connect(m_control.data(), SIGNAL(toggled(bool)), this, SLOT(slotButtonChecked(bool)));

    // allow Bool properties only
    if (m_property.type() != QVariant::Bool)
        qWarning("PE_AbstractSlider: unhandled property '%s' of type %d", propertyName, (int)m_property.type());
    else
        m_isValid = true;
}

void PE_AbstractButton::slotButtonChecked(bool boolValue)
{
    // set the property to the current state of the button
    if (m_control && m_target && (m_property.type() == QVariant::Int || m_property.type() == QVariant::Bool))
        m_property.write(m_target.data(), boolValue);
}

void PE_AbstractButton::slotPropertyChanged()
{
    // set the button check state as the bool property
    if (m_control && m_target && (m_property.type() == QVariant::Int || m_property.type() == QVariant::Bool)) {
        bool boolValue = m_property.read(m_target.data()).toBool();
        m_control->setChecked(boolValue);
    }
}



PE_Combo::PE_Combo(QComboBox * _combo, QObject * _target, const char * propertyName, QObject * parent)
  : PE_TypeControl<QComboBox>(_combo, _target, propertyName, parent)
{
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
    connect(m_control.data(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboChanged(int)));

    // allow Bool properties only
    if (m_property.type() != QVariant::Int)
        qWarning("PE_Combo: unhandled property '%s' of type %d", propertyName, (int)m_property.type());
    else
        m_isValid = true;
}

void PE_Combo::slotComboChanged(int intValue)
{
    // set the property to the current state of the button
    if (m_control && m_target && (m_property.type() == QVariant::Int))
        m_property.write(m_target.data(), intValue);
}

void PE_Combo::slotPropertyChanged()
{
    // set the button check state as the bool property
    if (m_control && m_target && (m_property.type() == QVariant::Int)) {
        int intValue = m_property.read(m_target.data()).toInt();
        m_control->setCurrentIndex(intValue);
    }
}
