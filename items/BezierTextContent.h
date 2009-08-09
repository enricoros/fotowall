#ifndef __BezierTextContent__
#define __BezierTextContent__

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetricsF>
#include <QDialog>
#include "AbstractContent.h"


class BezierTextContent : public AbstractContent
{
    private:
        QString m_text;
        QFont m_font;
        int m_fontSize;
        QPainterPath m_path;

    public:
        BezierTextContent(QGraphicsScene *scene, QGraphicsItem *parent = 0);
        void setText(const QString &text);
        QString text();
        void setFont(const QFont &font);
        void setPath(const QPainterPath &path);

        void configureBezier();

        // ::AbstractContent
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        QPixmap renderAsBackground(const QSize & size, bool keepAspect) const;
        int contentHeightForWidth(int width) const;

        // ::QGraphicsItem
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
};

#endif
