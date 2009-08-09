#ifndef __BezierTextDialog__
#define __BezierTextDialog__

#include <QDialog>
#include "ui_BezierTextDialog.h"

class QFont;
class QPainterPath;

class BezierTextDialog : public QDialog
{
    public:
        BezierTextDialog(QWidget *parent = 0);
        QFont font();
        QPainterPath path();
        int fontSize();
        QString text();

    private:
        Ui::BezierTextDialog *ui;
};

#endif
