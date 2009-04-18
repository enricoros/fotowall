/****************************************************************************
**
** Copyright (C) 2007-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Graphics Dojo project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "GlowEffectWidget.h"

#include <QPainter>
#include <QPainterPath>

#include <cmath>

// Exponential blur, Jani Huhtanen, 2006
//
template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha);

template<int aprec,int zprec>
static inline void blurrow( QImage & im, int line, int alpha);

template<int aprec, int zprec>
static inline void blurcol( QImage & im, int col, int alpha);

/*
*  expblur(QImage &img, int radius)
*
*  In-place blur of image 'img' with kernel
*  of approximate radius 'radius'.
*
*  Blurs with two sided exponential impulse
*  response.
*
*  aprec = precision of alpha parameter
*  in fixed-point format 0.aprec
*
*  zprec = precision of state parameters
*  zR,zG,zB and zA in fp format 8.zprec
*/
template<int aprec,int zprec>
void expblur( QImage &img, int radius )
{
  if(radius<1)
    return;

  /* Calculate the alpha such that 90% of
     the kernel is within the radius.
     (Kernel extends to infinity)
  */
  int alpha = (int)((1<<aprec)*(1.0f-expf(-2.3f/(radius+1.f))));

  for(int row=0;row<img.height();row++)
  {
    blurrow<aprec,zprec>(img,row,alpha);
  }

  for(int col=0;col<img.width();col++)
  {
    blurcol<aprec,zprec>(img,col,alpha);
  }
  return;
}

template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
  int R,G,B,A;
  R = *bptr;
  G = *(bptr+1);
  B = *(bptr+2);
  A = *(bptr+3);

  zR += (alpha * ((R<<zprec)-zR))>>aprec;
  zG += (alpha * ((G<<zprec)-zG))>>aprec;
  zB += (alpha * ((B<<zprec)-zB))>>aprec;
  zA += (alpha * ((A<<zprec)-zA))>>aprec;

  *bptr =     zR>>zprec;
  *(bptr+1) = zG>>zprec;
  *(bptr+2) = zB>>zprec;
  *(bptr+3) = zA>>zprec;
}

template<int aprec,int zprec>
static inline void blurrow( QImage & im, int line, int alpha)
{
  int zR,zG,zB,zA;

  QRgb *ptr = (QRgb *)im.scanLine(line);

  zR = *((unsigned char *)ptr    )<<zprec;
  zG = *((unsigned char *)ptr + 1)<<zprec;
  zB = *((unsigned char *)ptr + 2)<<zprec;
  zA = *((unsigned char *)ptr + 3)<<zprec;

  for(int index=1; index<im.width(); index++)
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
  for(int index=im.width()-2; index>=0; index--)
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }


}

template<int aprec, int zprec>
static inline void blurcol( QImage & im, int col, int alpha)
{
  int zR,zG,zB,zA;

  QRgb *ptr = (QRgb *)im.bits();
  ptr+=col;

  zR = *((unsigned char *)ptr    )<<zprec;
  zG = *((unsigned char *)ptr + 1)<<zprec;
  zB = *((unsigned char *)ptr + 2)<<zprec;
  zA = *((unsigned char *)ptr + 3)<<zprec;

  for(int index=im.width(); index<(im.height()-1)*im.width(); index+=im.width())
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }

  for(int index=(im.height()-2)*im.width(); index>=0; index-=im.width())
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }

}

// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
void fastbluralpha(QImage &img, int radius)
{
    if (radius < 1) {
        return;
    }

    QRgb *pix = (QRgb*)img.bits();
    int w   = img.width();
    int h   = img.height();
    int wm  = w-1;
    int hm  = h-1;
    int wh  = w*h;
    int div = radius+radius+1;

    int *r = new int[wh];
    int *g = new int[wh];
    int *b = new int[wh];
    int *a = new int[wh];
    int rsum, gsum, bsum, asum, x, y, i, yp, yi, yw;
    QRgb p;
    int *vmin = new int[qMax(w,h)];

    int divsum = (div+1)>>1;
    divsum *= divsum;
    int *dv = new int[256*divsum];
    for (i=0; i < 256*divsum; ++i) {
        dv[i] = (i/divsum);
    }

    yw = yi = 0;

    int **stack = new int*[div];
    for(int i = 0; i < div; ++i) {
        stack[i] = new int[4];
    }


    int stackpointer;
    int stackstart;
    int *sir;
    int rbs;
    int r1 = radius+1;
    int routsum, goutsum, boutsum, aoutsum;
    int rinsum, ginsum, binsum, ainsum;

    for (y = 0; y < h; ++y){
        rinsum = ginsum = binsum = ainsum
               = routsum = goutsum = boutsum = aoutsum
               = rsum = gsum = bsum = asum = 0;
        for(i =- radius; i <= radius; ++i) {
            p = pix[yi+qMin(wm,qMax(i,0))];
            sir = stack[i+radius];
            sir[0] = qRed(p);
            sir[1] = qGreen(p);
            sir[2] = qBlue(p);
            sir[3] = qAlpha(p);

            rbs = r1-abs(i);
            rsum += sir[0]*rbs;
            gsum += sir[1]*rbs;
            bsum += sir[2]*rbs;
            asum += sir[3]*rbs;

            if (i > 0){
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }
        }
        stackpointer = radius;

        for (x=0; x < w; ++x) {

            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];
            a[yi] = dv[asum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer-radius+div;
            sir = stack[stackstart%div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (y == 0) {
                vmin[x] = qMin(x+radius+1,wm);
            }
            p = pix[yw+vmin[x]];

            sir[0] = qRed(p);
            sir[1] = qGreen(p);
            sir[2] = qBlue(p);
            sir[3] = qAlpha(p);

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer+1)%div;
            sir = stack[(stackpointer)%div];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            ++yi;
        }
        yw += w;
    }
    for (x=0; x < w; ++x){
        rinsum = ginsum = binsum = ainsum
               = routsum = goutsum = boutsum = aoutsum
               = rsum = gsum = bsum = asum = 0;

        yp =- radius * w;

        for(i=-radius; i <= radius; ++i) {
            yi=qMax(0,yp)+x;

            sir = stack[i+radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];
            sir[3] = a[yi];

            rbs = r1-abs(i);

            rsum += r[yi]*rbs;
            gsum += g[yi]*rbs;
            bsum += b[yi]*rbs;
            asum += a[yi]*rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }

            if (i < hm){
                yp += w;
            }
        }

        yi = x;
        stackpointer = radius;

        for (y=0; y < h; ++y){
            pix[yi] = qRgba(dv[rsum], dv[gsum], dv[bsum], dv[asum]);

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer-radius+div;
            sir = stack[stackstart%div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (x==0){
                vmin[y] = qMin(y+r1,hm)*w;
            }
            p = x+vmin[y];

            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];
            sir[3] = a[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer+1)%div;
            sir = stack[stackpointer];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            yi += w;
        }
    }
    delete [] r;
    delete [] g;
    delete [] b;
    delete [] a;
    delete [] vmin;
    delete [] dv;
}

GlowEffectWidget::GlowEffectWidget(QWidget *parent)
    : QWidget(parent), m_radius(5)
{
    m_mouseIn   = true;
    m_mouseDown = false;

    m_tile = QPixmap(100, 100);
    m_tile.fill(Qt::white);
    QPainter pt(&m_tile);
    QColor color(240, 240, 240);
    pt.fillRect(0, 0, 50, 50, color);
    pt.fillRect(50, 50, 50, 50, color);
    pt.end();
    generateLens(QRectF(0, 0, 80, 80));
}

void GlowEffectWidget::setPreviewImage(const QImage & preview)
{
    m_image = preview.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setFixedSize(m_image.size());
    update();
}

template<class T>
inline const T& qClamp(const T &x, const T &low, const T &high)
{
    if      (x <  low) return low;
    else if (x > high) return high;
    else               return x;
}

void GlowEffectWidget::paintEvent(QPaintEvent *e)
{
    QImage back(size(), QImage::Format_ARGB32_Premultiplied);
    back.fill(0x00);
    QPainter p(&back);
    p.setClipRect(e->rect());

    p.drawTiledPixmap(rect(), m_tile);

    QImage blurred = m_image;

    // FIXME: what's the half-difference supposed to do here?
    int x = 0; //(size().width()  - blurred.size().width())/2;
    int y = 0; //(size().height() - blurred.size().height())/2;

    QRectF circle(m_pos.x()-40, m_pos.y()-40,
                  80, 80);

    p.drawImage(qClamp(x, 0, x),
                qClamp(y, 0, y),
                m_image);

    if (m_mouseDown) {
        //fastbluralpha(blurred, m_radius);
        //ExpBlur with 0.16 fp for alpha and
        //8.7 fp for state parameters zR,zG,zB and zA
        expblur<16,7>(blurred, m_radius);

        p.save();
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawImage(qClamp(x, 0, x),
                    qClamp(y, 0, y),
                    blurred);
        p.restore();
    }
    p.end();

    QPainter painter(this);
    painter.drawImage(0, 0, back);

    drawRadius(&painter);

    painter.end();

}

QImage GlowEffectWidget::glow(const QImage &image, int radius) const
{
    QImage back(image.size(), QImage::Format_ARGB32_Premultiplied);
    back.fill(0x00);
    QPainter p(&back);

    QImage blurred = image;

    // FIXME: what's the half-difference supposed to do here?
    int x = 0; //(size().width() - blurred.size().width())/2;
    int y = 0; //(size().height() - blurred.size().height())/2;

    p.drawImage(qClamp(x, 0, x),
            qClamp(y, 0, y),
            blurred);

    //fastbluralpha(blurred, radius);
    //ExpBlur with 0.16 fp for alpha and
    //8.7 fp for state parameters zR,zG,zB and zA
    expblur<16,7>(blurred, radius);

    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.drawImage(qClamp(x, 0, x),
            qClamp(y, 0, y),
            blurred);
    p.restore();
    p.end();

    return back;
}

void GlowEffectWidget::mousePressEvent(QMouseEvent *e)
{
    m_mouseDown = true;
    if (e->button() == Qt::LeftButton) {
        m_pos = e->pos();
        m_mouseDown = true;
    }
    update();
}


void GlowEffectWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_pos = e->pos();
    //m_radius = int(50 * (pos.y()/rect().width()));
    update();
}


void GlowEffectWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_mouseDown = false;
    update();
}

void GlowEffectWidget::setGlowRadius(int radius)
{
    m_radius = qClamp(radius, 0, 99);
}

int GlowEffectWidget::glowRadius() const
{
    return m_radius;
}

void GlowEffectWidget::enterEvent(QEvent *)
{
    m_mouseIn = true;
    update();
}

void GlowEffectWidget::leaveEvent(QEvent *)
{
    m_mouseIn = false;
    update();
}

void GlowEffectWidget::generateLens(const QRectF &bounds)
{
    QPainter painter;

    m_lens = QImage(bounds.size().toSize(),
                    QImage::Format_ARGB32_Premultiplied);
    m_lens.fill(0);
    painter.begin(&m_lens);
    const qreal rad = 40;

    QRadialGradient gr(rad, rad, rad, 3 * rad / 5, 3 * rad / 5);
    gr.setColorAt(0.0, QColor(255, 255, 255, 191));
    gr.setColorAt(0.2, QColor(255, 255, 127, 191));
    gr.setColorAt(0.9, QColor(150, 150, 200, 63));
    gr.setColorAt(0.95, QColor(0, 0, 0, 127));
    gr.setColorAt(1, QColor(0, 0, 0, 0));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(gr);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(bounds);
}

void GlowEffectWidget::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0)
        ++m_radius;
    else
        --m_radius;
    setGlowRadius(m_radius);
    update();
}

void GlowEffectWidget::drawRadius(QPainter *p)
{
    p->save();
    p->setWindow(0, 0, 800, 600);
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(Qt::black));
    p->setBrush(QBrush(QColor(193, 193, 193, 127)));
    p->drawRoundRect(10, 540, 160, 50, 10, 20);

    QFont font("ComicSans", 16);
    font.setUnderline(true);
    p->setFont(font);
    p->drawText(20, 560, "Radius");

    font.setUnderline(false);
    font.setPointSize(14);
    p->setFont(font);
    p->drawText(30, 580,
                QString("Radius is: %1").arg(m_radius));

    p->restore();
}
