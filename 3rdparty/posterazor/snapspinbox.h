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

#ifndef SNAPSPINBOX_H
#define SNAPSPINBOX_H

#include <QDoubleSpinBox>

// SnapSpinBox adds one simple behavior to QDoubleSpinBox:
// When stepping (via buttons or mouse wheel), the value is set
// to the next integral value, minimum 1.
// Instead of de/incrementing it by the "stepBy" value.

class SnapSpinBox: public QDoubleSpinBox
{
    Q_OBJECT

public:
    SnapSpinBox(QWidget *parent);
    void setValue(double value);

protected:
    double m_originalValue; // This is not rounded
    QAbstractSpinBox::StepEnabled stepEnabled() const;

private slots:
    void handleLineEditTextEdited(const QString &text);
    void stepBy(int steps);

signals:
    void valueEdited(double value) const;
};

#endif // SNAPSPINBOX_H
