#include "CropingWidget.h"
#include "CPixmap.h"
#include <QRubberBand>
#include <QMouseEvent>
#include <QPainter>

CropingWidget::CropingWidget(QWidget *parent) : QWidget(parent)
{
    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_rubberBand->setGeometry(0,0,0,0);
}

void CropingWidget::setPixmap(CPixmap *pix)
{
    m_photo = pix;
    m_previewPixmap = m_photo->scaled(500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_previewRatio = m_photo->width()/m_previewPixmap.width();
    setFixedSize(m_previewPixmap.size());
}

void CropingWidget::mousePressEvent(QMouseEvent *event)
{
    m_rubberBand->show();
    m_origin = event->pos();
    m_rubberBand->setGeometry(QRect(m_origin, QSize()));
}

void CropingWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_rubberBand->setGeometry(QRect(m_origin, event->pos()).normalized());
}

void CropingWidget::mouseReleaseEvent(QMouseEvent *)
{
    // determine selection, for example using QRect::intersects()
    // and QRect::contains().
}

void CropingWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(m_previewPixmap.rect(), m_previewPixmap);
}

QRect CropingWidget::getCropingRect() const
{
    QRectF selectionRect = m_rubberBand->geometry();
    return QRect(selectionRect.x()*m_previewRatio, selectionRect.y()*m_previewRatio,
                 selectionRect.width()*m_previewRatio, selectionRect.height()*m_previewRatio);
}
