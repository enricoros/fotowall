#ifndef __CropingWidget__
#define __CropingWidget__

#include <QWidget>
class QRubberBand;
class CPixmap;

class CropingWidget : public QWidget {
    private:
        QRubberBand *m_rubberBand;
        CPixmap *m_photo;
        QPixmap m_previewPixmap;
        QPoint m_origin;
        float m_previewRatio;
    public:
        CropingWidget(QWidget *parent=0);
        void setPixmap(CPixmap *pix);
        QRect getCropingRect() const;
    protected:
         void mousePressEvent(QMouseEvent *event);
         void mouseMoveEvent(QMouseEvent *event);
         void mouseReleaseEvent(QMouseEvent *event);
         void paintEvent(QPaintEvent *event);
};


#endif
