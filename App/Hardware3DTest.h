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

#ifndef __Hardware3DTest_h__
#define __Hardware3DTest_h__

#include <QDialog>
#include <QTime>
#include "SceneView.h"
class Canvas;
class TimedSceneView;
class PictureContent;

class Hardware3DTest : public QDialog
{
    Q_OBJECT
    public:
        Hardware3DTest(QWidget * parent = 0, Qt::WindowFlags f = 0);

        // run the software & hardware render tests
        enum ExitState { Canceled, UseSoftware, UseOpenGL };
        ExitState run();

    Q_SIGNALS:
        void testStarted();
        void testEnded(bool openglWins);

    private:
        Canvas * m_canvas;
        TimedSceneView * m_view;
        QVector<PictureContent *> m_pictures;
        QVector<int> m_results;
        int m_resultIdx;
        ExitState m_retCode;

        // state variables (TODO 1.0: replace with QStateMachine)
        enum State { Off, TestingSoftware, TestingOpenGL, Finished };
        State m_state;
        int m_statePhase;   // 0 .. TESTPOWER

    private Q_SLOTS:
        void showResults();
        void nextStep();
        void slotViewRepainted(int durationMs);
        void slotUseSoftware();
        void slotUseOpenGL();
};

class TimedSceneView : public SceneView
{
    Q_OBJECT
    public:
        TimedSceneView(QWidget * parent = 0);

        // start a measure, will emit 'repainted' upon completion
        void measureNextRepaint();

        // flush updats
        void flushPaints();

    Q_SIGNALS:
        void repainted(int durationMs);

    protected:
        // ::QWidget
        void paintEvent(QPaintEvent * event);

    private:
        bool m_measureRepaint;
};

#endif
