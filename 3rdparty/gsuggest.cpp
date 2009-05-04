/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "ui_search.h"
#include "dragmovecharm.h"

#define GSEARCH_URL "http://www.google.it/search?q=%1"
#define GSUGGEST_URL "http://google.it/complete/search?output=toolbar&q=%1"

class GSuggestCompletion : public QObject
{
    Q_OBJECT

private:
    QLineEdit *editor;
    QTreeWidget *popup;
    QTimer *timer;
    QNetworkAccessManager networkManager;

public:
    GSuggestCompletion(QLineEdit *parent = 0): QObject(parent), editor(parent) {

        popup = new QTreeWidget;
        popup->setColumnCount(2);
        popup->setUniformRowHeights(true);
        popup->setRootIsDecorated(false);
        popup->setEditTriggers(QTreeWidget::NoEditTriggers);
        popup->setSelectionBehavior(QTreeWidget::SelectRows);
        popup->setFrameStyle(QFrame::Box | QFrame::Plain);
        popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        popup->header()->hide();
        popup->installEventFilter(this);
        popup->setMouseTracking(true);

        connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
                SLOT(doneCompletion()));

        popup->setWindowFlags(Qt::Popup);
        popup->setFocusPolicy(Qt::NoFocus);
        popup->setFocusProxy(parent);

        timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(500);
        connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
        connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));

        connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(handleNetworkData(QNetworkReply*)));

    }

    ~GSuggestCompletion() {
        delete popup;
    }

    bool eventFilter(QObject *obj, QEvent *ev) {
        if (obj != popup)
            return false;

        if (ev->type() == QEvent::MouseButtonPress) {
            popup->hide();
            editor->setFocus();
            return true;
        }

        if (ev->type() == QEvent::KeyPress) {

            bool consumed = false;
            int key = static_cast<QKeyEvent*>(ev)->key();
            switch (key) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                doneCompletion();
                consumed = true;

            case Qt::Key_Escape:
                editor->setFocus();
                popup->hide();
                consumed = true;

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
                break;

            default:
                editor->setFocus();
                editor->event(ev);
                popup->hide();
                break;
            }

            return consumed;
        }

        return false;
    }

    void showCompletion(const QStringList &choices, const QStringList &hits) {

        if (choices.isEmpty() || choices.count() != hits.count())
            return;

        const QPalette &pal = editor->palette();
        QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

        popup->setUpdatesEnabled(false);
        popup->clear();
        for (int i = 0; i < choices.count(); ++i) {
            QTreeWidgetItem * item;
            item = new QTreeWidgetItem(popup);
            item->setText(0, choices[i]);
            item->setText(1, hits[i]);
            item->setTextAlignment(1, Qt::AlignRight);
            item->setTextColor(1, color);
        }
        popup->setCurrentItem(popup->topLevelItem(0));
        popup->resizeColumnToContents(0);
        popup->resizeColumnToContents(1);
        popup->adjustSize();
        popup->setUpdatesEnabled(true);

        int h = popup->sizeHintForRow(0) * qMin(7, choices.count()) + 3;
        popup->resize(popup->width(), h);

        popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
        popup->setFocus();
        popup->show();
    }

public slots:

    void doneCompletion() {
        timer->stop();
        popup->hide();
        editor->setFocus();
        QTreeWidgetItem *item = popup->currentItem();
        if (item) {
            editor->setText(item->text(0));
            QKeyEvent *e;
            e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
            QApplication::postEvent(editor, e);
            e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
            QApplication::postEvent(editor, e);
        }
    }

    void preventSuggest() {
        timer->stop();
    }

    void autoSuggest() {
        QString str = editor->text();
        QString url = QString(GSUGGEST_URL).arg(str);
        networkManager.get(QNetworkRequest(QString(url)));
    }

    void handleNetworkData(QNetworkReply *networkReply) {
        QUrl url = networkReply->url();
        if (!networkReply->error()) {
            QStringList choices;
            QStringList hits;

            QString response(networkReply->readAll());
            QXmlStreamReader xml(response);
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.tokenType() == QXmlStreamReader::StartElement)
                    if (xml.name() == "suggestion") {
                        QStringRef str = xml.attributes().value("data");
                        choices << str.toString();
                    }
                if (xml.tokenType() == QXmlStreamReader::StartElement)
                    if (xml.name() == "num_queries") {
                        QStringRef str = xml.attributes().value("int");
                        hits << str.toString();
                    }
            }

            showCompletion(choices, hits);
        }

        networkReply->deleteLater();
    }


};


class SearchBox: public QFrame
{
    Q_OBJECT

private:
    Ui_searchForm ui;
    GSuggestCompletion *completion;

public:
    SearchBox(QWidget *parent = 0): QFrame(parent) {
        ui.setupUi(this);
        setFrameStyle(QFrame::StyledPanel + QFrame::Raised);

        completion = new GSuggestCompletion(ui.searchEdit);

        connect(ui.searchEdit, SIGNAL(returnPressed()), SLOT(doSearch()));
        connect(ui.searchButton, SIGNAL(clicked()), SLOT(doSearch()));

        adjustSize();
        resize(400, height());
        ui.searchEdit->setFocus();
    }

public slots:

    void doSearch() {
        completion->preventSuggest();
        QString str = ui.searchEdit->text();
        QString url = QString(GSEARCH_URL).arg(str);
        QDesktopServices::openUrl(QUrl(url));
    }

protected:

    void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Escape)
            close();
    }

};

#include "gsuggest.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SearchBox suggest;
    suggest.setWindowFlags(Qt::FramelessWindowHint);
    suggest.show();

    DragMoveCharm charm;
    charm.activateOn(&suggest);

    return app.exec();
}

