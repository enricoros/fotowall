#include "BezierTextDialog.h"

BezierTextDialog::BezierTextDialog(QWidget *parent) : QDialog (parent)
{
    ui = new Ui::BezierTextDialog();
    ui->setupUi(this);
}
