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

#include "Hardware3DTest.h"

#include "Canvas/Canvas.h"
#include "Canvas/PictureContent.h"
#include "Shared/RenderOpts.h"

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QGraphicsView>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>


#define FIXEDSIZE QSize(320, 240)
#define TESTPOWER 5
#define TESTSIZE (1 << TESTPOWER)

Hardware3DTest::Hardware3DTest(QWidget * parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
  , m_canvas(new Canvas(FIXEDSIZE))
  , m_view(0)
  , m_resultIdx(0)
  , m_retCode(Canceled)
  , m_state(Off)
  , m_statePhase(0)
{
    // customize dialog
    setWindowTitle(tr("OpenGL Test"));
    setWindowModality(Qt::ApplicationModal);
    setLayout(new QVBoxLayout);
    layout()->setAlignment(Qt::AlignCenter);
    layout()->setMargin(0);

    // create the timed SceneView
    m_view = new TimedSceneView(this);
    connect(m_view, SIGNAL(repainted(int)), this, SLOT(slotViewRepainted(int)), Qt::QueuedConnection);
    layout()->addWidget(m_view);

    // draw a fake contents pixmap
    QPixmap picturePix = QPixmap(500, 500);
    picturePix.fill(Qt::transparent);
    QPainter pixPainter(&picturePix);
    QRadialGradient gradient(200, 200, 300, 0, 0);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 200));
    gradient.setColorAt(1.0, QColor(0, 0, 0, 100));
    pixPainter.fillRect(picturePix.rect(), gradient);
    pixPainter.end();

    // populate scene with PictureContents
    m_pictures.resize(TESTSIZE);
    m_results.resize((TESTPOWER + 1) * 2);
    for (int i = 0; i < TESTSIZE; i++) {
        PictureContent * p = new PictureContent(m_canvas);
        p->loadPixmap(picturePix);
        p->setRotation(-60 + (qrand() % 120));
        p->setOpacity((qreal)(qrand() % 100) / 99.0);
        m_canvas->addManualContent(p, QPoint(qrand() % FIXEDSIZE.width(), qrand() % FIXEDSIZE.height()));
        p->hide();
        m_pictures[i] = p;
    }
    m_view->flushPaints();
}

Hardware3DTest::ExitState Hardware3DTest::run()
{
    // ensure we're layouted before applying the canvas
    show();
    m_view->setScene(m_canvas);

    // start testing
    nextStep();

    // run with the events
    if (exec() == QDialog::Rejected)
        return Canceled;
    return m_retCode;
}

void Hardware3DTest::showResults()
{
    // delete previous contents
    delete m_canvas;
    m_canvas = 0;
    delete m_view;
    m_view = 0;

    // drawing consts
    static const int margin = 25;
    static const int radius = 4;
    static const int colWidth = 50;
    static const int cols = TESTPOWER + 1;
    static const int colsWidth = colWidth * (cols - 1);
    static const int rowsHeight = 150;
    static const int graphWidth = colsWidth + 2 * (radius + margin);
    static const int graphHeight = rowsHeight + 2 * (radius + margin);

    // find out min/max of the series
    int max = 0;
    int min = 0;
    for (int i = 0; i < m_results.size(); ++i) {
        if (m_results[i] > max || !max)
            max = m_results[i];
        // commented to let the minimum be zero
        //if (m_results[i] < min || !min)
        //    min = m_results[i];
    }
    if (max <= min)
        max = min + 1;

    // draw the graph
    #define DATAPOINT(x, y) \
        QPointF(x * colWidth + margin + radius, graphHeight - radius - margin - (y - min) * rowsHeight / (max - min))
    QPixmap pix(graphWidth, graphHeight);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);
    QLinearGradient lg(0, 0, 0, graphHeight);
    lg.setColorAt(0.0, QColor(250, 250, 250));
    lg.setColorAt(1.0, Qt::transparent);
    p.fillRect(pix.rect(), lg);
    int seriesSum[2];
    for (int s = 0; s < 2; s++) {
        seriesSum[s] = 0;
        int dIdx = s * cols;
        int val = m_results[dIdx + 0];
        QColor col = s ? Qt::darkGreen : Qt::blue;

        // draw lines
        QPointF from = DATAPOINT(0, val);
        p.setPen(col);
        p.setBrush(Qt::NoBrush);
        for (int c = 1; c < cols; c++) {
            val = m_results[dIdx + c];
            QPointF to = DATAPOINT(c, val);
            p.drawLine(from, to);
            from = to;
        }

        // draw circles
        p.setPen(Qt::black);
        p.setBrush(col);
        for (int c = 0; c < cols; c++) {
            val = m_results[dIdx + c];
            QPointF to = DATAPOINT(c, val);
            p.drawEllipse(to, radius, radius);
            seriesSum[s] += val;
        }
    }
    p.setBrush(Qt::NoBrush);

    p.setPen(Qt::blue);
    p.drawText(margin + radius, margin - 15, colsWidth, 20, 0, tr("Non-OpenGL"));
    p.setPen(Qt::darkGreen);
    p.drawText(margin + radius, margin - 15 + 20, colsWidth, 20, 0, tr("OpenGL"));

    p.setPen(Qt::darkGray);
    p.drawText(margin + radius, margin + radius + rowsHeight + radius, colsWidth, 20, Qt::AlignCenter, tr("samples (%1...%2)").arg(1).arg(TESTSIZE));
    p.translate(margin, margin + radius + rowsHeight);
    p.rotate(-90);
    p.drawText(0, -20, rowsHeight, 20, Qt::AlignCenter, tr("duration (%1...%2ms)").arg(min).arg(max));
    p.end();

    // display the new widgets
    QLabel * results = new QLabel(this);
     layout()->addWidget(results);
     results->setFixedSize(pix.size());
     results->setPixmap(pix);
     results->show();

    QPushButton * b1 = new QPushButton(tr("Use OpenGL"), this);
     connect(b1, SIGNAL(clicked()), this, SLOT(slotUseOpenGL()));
     layout()->addWidget(b1);

    QPushButton * b2 = new QPushButton(tr("Don't use OpenGL"), this);
     connect(b2, SIGNAL(clicked()), this, SLOT(slotUseSoftware()));
     layout()->addWidget(b2);

    QFont boldFont;
    boldFont.setBold(true);
    boldFont.setPointSize(boldFont.pointSize() + 2);
    if (seriesSum[0] > seriesSum[1]) {
        setWindowTitle(tr("OpenGL Won"));
        b1->setFont(boldFont);
        b1->setFocus();
    } else {
        setWindowTitle(tr("OpenGL Lost"));
        b2->setFont(boldFont);
        b2->setFocus();
    }

    adjustSize();
}

void Hardware3DTest::nextStep()
{
    // advance state
    switch (m_state) {
        case Off:
            // go to Software test
            if (true) {
                m_state = TestingSoftware;
                m_statePhase = 0;
                m_view->setOpenGL(false);
            }
            break;

        case TestingSoftware:
            // eventually go to Hardware test
            if (++m_statePhase > TESTPOWER) {
                m_state = TestingOpenGL;
                m_statePhase = 0;
                m_view->setOpenGL(true);
            }
            break;

        case TestingOpenGL:
            // eventually go to Final state
            if (++m_statePhase > TESTPOWER) {
                showResults();
                m_state = Finished;
                m_statePhase = 0;
                return;
            }
            break;

        case Finished:
            return;
    }

    // apply state configuration
    int visibleCount = 1 << m_statePhase;
    for (int i = 0; i < TESTSIZE; i++)
        m_pictures[i]->setVisible(i <= visibleCount);

    // go measure
    m_view->measureNextRepaint();
}

void Hardware3DTest::slotViewRepainted(int durationMs)
{
    // got the measure
    m_results[m_resultIdx++] = durationMs;
    nextStep();
}

void Hardware3DTest::slotUseSoftware()
{
    m_retCode = UseSoftware;
    accept();
}

void Hardware3DTest::slotUseOpenGL()
{
    m_retCode = UseOpenGL;
    accept();
}


TimedSceneView::TimedSceneView(QWidget * parent)
  : SceneView(parent)
  , m_measureRepaint(false)
{
    // non interactive, fixed sizes
    setInteractive(false);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setFixedSize(FIXEDSIZE);
}

void TimedSceneView::measureNextRepaint()
{
    flushPaints();
    m_measureRepaint = true;
    viewport()->update();
}

void TimedSceneView::flushPaints()
{
    // flush out pending painting events
    viewport()->update();
    QApplication::processEvents();
    QApplication::processEvents();
}

void TimedSceneView::paintEvent(QPaintEvent * event)
{
    // start time measurement if needed
    QTime * time = 0;
    if (m_measureRepaint && event->rect().size() == FIXEDSIZE) {
        time = new QTime;
        time->start();
    }

    // do painting
    SceneView::paintEvent(event);

    // get measurement results
    if (time) {
        int elapsed = time->elapsed();
        delete time;
        m_measureRepaint = false;
        emit repainted(elapsed);
    }
}
