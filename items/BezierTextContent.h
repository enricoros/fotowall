#ifndef __BezierTextContent__
#define __BezierTextContent__

class BezierTextContent : public AbstractContent
{
    private:
        QString m_text;
    public:
        BezierTextContent(QGraphicsScene *scene, QGraphicsItem *parent = 0);
        void setText(const QString &text);
        QString text();

        // ::AbstractContent
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        QPixmap renderAsBackground(const QSize & size, bool keepAspect) const;
        int contentHeightForWidth(int width) const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
}

#endif
