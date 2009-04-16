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

#include "Selection.h"
#include "items/PictureContent.h"
#include "items/TextContent.h"
#include "frames/FrameFactory.h"

Selection::Selection()
{
}

void Selection::select(AbstractContent *content)
{
    m_selectedContent << content;
}

void Selection::unselect(AbstractContent *content)
{
    m_selectedContent.removeOne(content);
}

void Selection::clearSelection()
{
    foreach(AbstractContent *content, m_selectedContent) {
        content->setSelected(false);
        m_selectedContent.removeOne(content);
    }
}

QList<AbstractContent *> Selection::getSelectedContent() const
{
    return m_selectedContent;
}




void Selection::slotApplyLookToSelection(quint32 frameClass, bool mirrored)
{
    foreach (AbstractContent * content, m_selectedContent) {
        content->setFrame(FrameFactory::createFrame(frameClass));
        content->setMirrorEnabled(mirrored);
    }
}

void Selection::slotApplyEffectToSelection(int effectClass)
{
    foreach (AbstractContent * content, m_selectedContent) {
        PictureContent * picture = dynamic_cast<PictureContent *>(content);
        if (picture)
            picture->setEffect(effectClass);
    }
}

void Selection::slotReflexionToogled(bool state)
{
    foreach(AbstractContent *content, m_selectedContent) {
        content->setMirrorEnabled(state);
    }
}
void Selection::slotFlipHorizontally()
{
    foreach(AbstractContent *content, m_selectedContent) {
        PictureContent *picture = 0;
        picture = dynamic_cast<PictureContent *>(content);
        if (picture) {
            picture->flipH();
        }
    }
}
void Selection::slotFlipVertically()
{
    foreach(AbstractContent *content, m_selectedContent) {
        PictureContent *picture = 0;
        picture = dynamic_cast<PictureContent *>(content);
        if (picture) {
            picture->flipV();
        }
    }
}
