/*
    PosteRazor - Make your own poster!
    Copyright (C) 2005-2009 by Alessandro Portale
    http://posterazor.sourceforge.net/

    This file is part of PosteRazor

    PosteRazor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PosteRazor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PosteRazor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "snapspinbox.h"

SnapSpinBox::SnapSpinBox(QWidget *parent)
:    QDoubleSpinBox(parent)
,    m_originalValue(0.0)
{
    connect((const QObject *)lineEdit(), SIGNAL(textEdited(const QString &)), this, SLOT(handleLineEditTextEdited(const QString &)));
}

QAbstractSpinBox::StepEnabled SnapSpinBox::stepEnabled() const
{
    QAbstractSpinBox::StepEnabled enabled = QAbstractSpinBox::StepUpEnabled;
    if (value() > 1.0)
        enabled |= QAbstractSpinBox::StepDownEnabled;
    return enabled;
}

void SnapSpinBox::setValue(double value)
{
    // We intercept setValue() in order to always have the non-rounded value
    m_originalValue = value;
    QDoubleSpinBox::setValue(value);
}

void SnapSpinBox::handleLineEditTextEdited(const QString &text)
{
    m_originalValue = text.toDouble();
    emit(valueEdited(m_originalValue));
}

void SnapSpinBox::stepBy(int steps)
{
    double increaseValue = (steps > 0)?(steps-.5):(steps+.49999);
    double oldValue = m_originalValue;
    double newValue = qRound(oldValue + increaseValue);

    if (newValue < 1)
        return;

    setValue(newValue);
    emit(valueEdited(value()));
}
