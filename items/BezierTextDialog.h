#ifndef __BezierTextDialog__
#define __BezierTextDialog__

#include <QDialog>
#include "ui_BezierTextDialog.h"

class BezierTextDialog : public QDialog
{
    public:
        BezierTextDialog(QWidget *parent = 0);
    private:
    Ui::BezierTextDialog *ui;
};

#endif
