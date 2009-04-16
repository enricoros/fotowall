/*****************************************************************************
 * Copyright (C) 2008 TANGUY Arnaud <arn.tanguy@gmail.com>                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    * 
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                *
 ******************************************************************************/

#ifndef __Selection__
#define __Selection__

#include <QList>
#include <QObject>

class AbstractContent;

/*
 * \brief : This class manages the selected items
 * It contains the list of selected items, and provide slots to interact with them */
class Selection : public QObject
{
    Q_OBJECT
    public:
        Selection();
        void select(AbstractContent *);
        void unselect(AbstractContent *);
        void clearSelection();
        QList<AbstractContent *> getSelectedContent() const;

    private:
        QList<AbstractContent *> m_selectedContent;

    public Q_SLOTS:
        void slotApplyLookToSelection(quint32 frameClass, bool mirrored);
        void slotApplyEffectToSelection(int effectClass);
        void slotReflexionToogled(bool);
        void slotFlipVertically();
        void slotFlipHorizontally();

};


#endif 

