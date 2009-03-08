#include <QtGui>
class SizeDialog :  public QDialog {
	Q_OBJECT
    public:
        SizeDialog(QWidget * parent = 0)
            : QDialog(parent)
        {
            setWindowTitle(tr("Select Resolution"));

            wSpin = new QSpinBox(this);
            wSpin->setRange(100, 10000);
            hSpin = new QSpinBox(this);
            hSpin->setRange(100, 10000);
            QPushButton * closeButton = new QPushButton(tr("OK"), this);
            connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

            QHBoxLayout * lay = new QHBoxLayout(this);
            lay->addWidget(wSpin);
            lay->addWidget(hSpin);
            lay->addWidget(closeButton);
        }
        QSpinBox * wSpin;
        QSpinBox * hSpin;
};

