#include "BezierTextDialog.h"

BezierTextDialog::BezierTextDialog(QWidget *parent) : QDialog (parent)
{
    ui = new Ui::BezierTextDialog();
    ui->setupUi(this);
}

QFont BezierTextDialog::font()
{
    return ui->BezierWidget->font();
}

QPainterPath BezierTextDialog::path()
{
    return ui->BezierWidget->path();
}

int BezierTextDialog::fontSize()
{
    return ui->BezierWidget->fontSize();
}

QString BezierTextDialog::text()
{
    return ui->BezierWidget->text();
}
