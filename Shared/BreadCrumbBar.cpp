/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>        *
 *   Started on 23 Sep 2009 by root.
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "BreadCrumbBar.h"

#include <QApplication>
#include <QFont>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>

#define BAR_RADIUS 6
#define BAR_H_MARGIN 7
#define BAR_V_MARGIN 2

/// BcLabel

BcLabel::BcLabel(quint32 labelId, QWidget * parent)
  : QLabel(parent)
  , m_labId(labelId)
  , m_last(true)
  , m_hover(false)
{
    // shrink font
    QFont font;
    font.setPointSize(font.pointSize() - 1);
    setFont(font);
}

void BcLabel::setLast(bool last)
{
    m_last = last;
    QPalette pal;
    pal.setBrush(QPalette::Text, m_last ? Qt::darkRed : Qt::darkGray);
    setPalette(pal);
    setCursor(m_last ? Qt::ArrowCursor : Qt::PointingHandCursor);
    update();
}

bool BcLabel::last() const
{
    return m_last;
}

void BcLabel::enterEvent(QEvent * /*event*/)
{
    m_hover = true;
    update();
}

void BcLabel::leaveEvent(QEvent * /*event*/)
{
    m_hover = false;
    update();
}

void BcLabel::mousePressEvent(QMouseEvent * event)
{
    if (!m_last && event->button() == Qt::LeftButton) {
        event->accept();
        emit labelClicked(m_labId);
    }
}

void BcLabel::paintEvent(QPaintEvent * event)
{
    // draw underlining if clickable
    if (!m_last && m_hover) {
        QPainter p(this);
        p.setPen(QPen(QPalette().highlight().color(), 1));
        p.drawLine(0, height() - 2, width(), height() - 2);
    }

    // unbreak painting
    QLabel::paintEvent(event);
}


/// BcExpander

BcExpander::BcExpander(quint32 expanderId, QWidget * parent)
  : QWidget(parent)
  , m_exId(expanderId)
  , m_count(1)
{
    setFixedSize(11, 11);
}

void BcExpander::setCount(int count)
{
    m_count = count;
    setCursor(count > 1 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    update();
}

int BcExpander::count() const
{
    return m_count;
}

void BcExpander::mousePressEvent(QMouseEvent * event)
{
    if (m_count > 1 && event->button() == Qt::LeftButton)
        emit expanderClicked(m_exId);
}

void BcExpander::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::lightGray, 2));
    if (m_count > 1) {
        if (QApplication::isRightToLeft()) {
            p.drawLine(5, 1, 1, 5);
            p.drawLine(1, 5, 5, 9);
            p.drawLine(5, 9, 9, 5);
        } else {
            p.drawLine(5, 1, 9, 5);
            p.drawLine(9, 5, 5, 9);
            p.drawLine(5, 9, 1, 5);
        }
    } else {
        if (QApplication::isRightToLeft()) {
            p.drawLine(7, 1, 3, 5);
            p.drawLine(3, 5, 7, 9);
        } else {
            p.drawLine(3, 1, 7, 5);
            p.drawLine(7, 5, 3, 9);
        }
    }
}


/// Internal Node (BreadCrumbBar)

struct InternalNode {
    quint32 id;
    QString text;
    InternalNode * parent;
    QList<InternalNode *> children;

    BcLabel * label;
    BcExpander * expander;

    InternalNode(quint32 id, const QString & text)
      : id(id)
      , text(text)
      , parent(0)
      , label(0)
      , expander(0)
    {
    }
    ~InternalNode()
    {
        if (parent) {
            parent->children.removeAll(this);
            parent->clearWidgetry();
        }
        qDeleteAll(children);
        delete expander;
        delete label;
    }
    InternalNode * findNode(quint32 searchId) {
        if (id == searchId)
            return this;
        foreach (InternalNode * child, children)
            if (InternalNode * cNode = child->findNode(searchId))
                return cNode;
        return 0;
    }
    void clearWidgetry() {
        delete expander;
        expander = 0;
        delete label;
        label = 0;
        foreach (InternalNode * child, children)
            child->clearWidgetry();
    }
};


/// BreadCrumbBar

BreadCrumbBar::BreadCrumbBar(QWidget * parent)
  : QWidget(parent)
  , m_root(0)
  , m_translucent(false)
{
    // init defaults
    processLayout();
}

quint32 BreadCrumbBar::addNode(quint32 id, const QString & text, quint32 parentId)
{
    // sanity check: duplicated id
    if (m_root && m_root->findNode(id)) {
        qWarning("BreadCrumbBar::addNode: already present a node with id 0x%x. skipping", id);
        return InvalidNode;
    }

    // find out where to insert the node
    InternalNode * parentNode = (parentId == InvalidNode) ? 0 : (m_root ? m_root->findNode(parentId) : 0);
    if (m_root && !parentNode) {
        qWarning("BreadCrumbBar::addNode: can't add more than 1 node to the root");
        return InvalidNode;
    }

    // create the node
    InternalNode * node = new InternalNode(id, text);
    if (parentNode) {
        parentNode->children.append(node);
        node->parent = parentNode;
    }
    if (!m_root)
        m_root = node;
    processLayout();
    return id;
}

void BreadCrumbBar::deleteNode(quint32 id)
{
    if (!m_root)
        return;
    InternalNode * node = m_root->findNode(id);
    if (!node)
        return;
    delete node;
    if (node == m_root)
        m_root = 0;
    processLayout();
}

void BreadCrumbBar::clearNodes()
{
    delete m_root;
    m_root = 0;
    processLayout();
}

void BreadCrumbBar::paintEvent(QPaintEvent * event)
{
    // translucent painting
    if (m_translucent && layout()) {
#if 0
        return;
#else
        // find children boundaries
        QRectF boundaries;
        const QLayout * lay = layout();
        const int children = lay->count();
        for (int i = 0; i < children; ++i) {
            QLayoutItem * child = lay->itemAt(i);
            if (child->widget())
                boundaries = boundaries.isNull() ? child->geometry() : boundaries.united(child->geometry());
        }
        boundaries.adjust(-BAR_H_MARGIN + 0.5, -BAR_V_MARGIN + 0.5, BAR_H_MARGIN - 0.5, BAR_V_MARGIN - 0.5);

        // adapt a bit the boundaries to hide a round side
        if (QApplication::isRightToLeft())
            boundaries.adjust(0, 0, BAR_RADIUS, 0);
        else
            boundaries.adjust(-BAR_RADIUS, 0, 0, 0);

        // paint a rounded rect
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(Qt::darkGray, 1));
        QLinearGradient lg(0, 0, 0, height());
        lg.setColorAt(0.0, QColor(255, 255, 255));
        lg.setColorAt(1.0, QColor(200, 200, 200));
        p.setBrush(lg);
        p.drawRoundedRect(boundaries, BAR_RADIUS, BAR_RADIUS, Qt::AbsoluteSize);
#endif
    } else {
        QPainter p(this);
        QLinearGradient lg(0, 0, 0, height());
        lg.setColorAt(0.0, QColor(237, 237, 237));
        lg.setColorAt(1.0, Qt::lightGray);
        p.fillRect(event->rect(), lg);
    }
}

void BreadCrumbBar::setTranslucent(bool translucent)
{
    if (m_translucent != translucent) {
        m_translucent = translucent;
        update();
    }
}

bool BreadCrumbBar::translucent() const
{
    return m_translucent;
}

void BreadCrumbBar::processLayout()
{
    // recreate the layout
    delete layout();
    QHBoxLayout * hLay = new QHBoxLayout(this);
    hLay->setContentsMargins(BAR_H_MARGIN, BAR_V_MARGIN, BAR_H_MARGIN, BAR_V_MARGIN);
    hLay->setSpacing(6);
    setLayout(hLay);

    // hide if empty
    if (!m_root) {
        hide();
        return;
    }
    show();

    // build up the layout (and widgets too, if needed)
    InternalNode * node = m_root;
    while (node) {
        // create label if missing
        if (!node->label) {
            node->label = new BcLabel(node->id, this);
            node->label->setText(node->text);
            connect(node->label, SIGNAL(labelClicked(quint32)), this, SLOT(slotLabelClicked(quint32)));
        }
        node->label->setLast(node->children.isEmpty());
        hLay->addWidget(node->label);

        // create/destroy expander when needed
        if (node->children.isEmpty()) {
            if (node->expander) {
                delete node->expander;
                node->expander = 0;
            }
            break;
        }
        if (!node->expander) {
            node->expander = new BcExpander(node->id, this);
            connect(node->expander, SIGNAL(expanderClicked(quint32)), this, SLOT(slotExpanderClicked(quint32)));
        }
        node->expander->setCount(node->children.size());
        hLay->addWidget(node->expander);

        // continue the chain
        node = node->children.first();
    }

    // add a stretch to the end
    hLay->addStretch(10);

    // repaint all
    update();
}

void BreadCrumbBar::slotLabelClicked(quint32 id)
{
    emit nodeClicked(id);
}

void BreadCrumbBar::slotExpanderClicked(quint32 parentId)
{
    InternalNode * pNode = m_root ? m_root->findNode(parentId) : 0;
    if (!pNode)
        return;

    // create menu
    QMenu menu;
    int listIndex = 0;
    foreach (InternalNode * child, pNode->children) {
        QAction * action = menu.addAction(child->text);
        if (!listIndex) {
            QFont boldFont;
            boldFont.setBold(true);
            action->setFont(boldFont);
        }
        action->setProperty("listIndex", listIndex++);
    }

    // show menu
    BcExpander * expander = static_cast<BcExpander *>(sender());
    QAction * action = menu.exec(expander->mapToGlobal(QPoint(0, expander->height())));
    if (!action)
        return;

    // change the branch
    listIndex = action->property("listIndex").toInt();
    if (listIndex > 0 && listIndex < pNode->children.size()) {
        pNode->children.first()->clearWidgetry();
        InternalNode * node = pNode->children.takeAt(listIndex);
        pNode->children.prepend(node);
        processLayout();
        emit nodeClicked(node->id);
    }
}
