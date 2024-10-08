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

#ifndef __TextContent_h__
#define __TextContent_h__

#include "AbstractContent.h"
class BezierCubicItem;
class BezierControlPoint;
class QTextDocument;

/// \brief TODO
class TextContent : public AbstractContent
{
  Q_OBJECT
  Q_PROPERTY(QString html READ toHtml WRITE setHtml)
  Q_PROPERTY(bool hasShape READ hasShape NOTIFY notifyHasShape)
  Q_PROPERTY(bool shapeEditing READ isShapeEditing WRITE setShapeEditing NOTIFY notifyShapeEditing)
  Q_PROPERTY(QPainterPath shapePath READ shapePath WRITE setShapePath)
public:
  TextContent(bool spontaneous, QGraphicsScene * scene);
  ~TextContent();

public Q_SLOTS:
  QString toHtml() const;
  void setHtml(const QString & htmlCode);
  void setPlainText(const QString & plainText);

  bool hasShape() const;
  void clearShape();

  bool isShapeEditing() const;
  void setShapeEditing(bool enabled);

  QPainterPath shapePath() const;
  void setShapePath(const QPainterPath & path);

  void slotControlPointChanged(const QList<QPointF> &);

Q_SIGNALS:
  void notifyHasShape(bool);
  void notifyShapeEditing(bool);

public:
  // ::AbstractContent
  QString contentName() const { return tr("Text"); }
  QWidget * createPropertyWidget(ContentProperties * __p = 0);
  bool fromXml(const QDomElement & contentElement, const QDir & baseDir);
  void toXml(QDomElement & contentElement, const QDir & baseDir) const;
  void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio);
  int contentHeightForWidth(int width) const;
  void selectionChanged(bool selected);

  void setControlPoints(const QList<QPointF> & cps);

  // ::QGraphicsItem
  void keyPressEvent(QKeyEvent * event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

private:
  void updateTextConstraints();
  void updateCache();

  // text document, layouting & rendering
  QTextDocument * m_text;
  QList<QRect> m_blockRects;
  QRect m_textRect;
  int m_textMargin;
  int m_shakeRadius;

  // shape related stuff
  BezierCubicItem * m_shapeEditor;
  QPainterPath m_shapePath;
  QList<QPointF> m_previousCps;
  QRect m_shapeRect;

private Q_SLOTS:
  void slotShakeLess();
  void slotShakeMore();
};

#endif
