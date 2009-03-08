/****************************************************************************
** Meta object code from reading C++ file 'FotoWall.h'
**
** Created: Fri Nov 14 22:44:31 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FotoWall.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FotoWall.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FotoWall[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      34,    9,    9,    9, 0x08,
      59,    9,    9,    9, 0x08,
      94,    9,    9,    9, 0x08,
     118,    9,    9,    9, 0x08,
     141,    9,    9,    9, 0x08,
     170,  165,    9,    9, 0x08,
     204,    9,    9,    9, 0x08,
     241,    9,    9,    9, 0x08,
     278,    9,    9,    9, 0x08,
     303,    9,    9,    9, 0x08,
     335,    9,    9,    9, 0x08,
     370,    9,    9,    9, 0x08,
     407,    9,    9,    9, 0x08,
     442,    9,    9,    9, 0x08,
     478,    9,    9,    9, 0x08,
     515,    9,    9,    9, 0x08,
     554,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FotoWall[] = {
    "FotoWall\0\0on_loadButton_clicked()\0"
    "on_addPictures_clicked()\0"
    "on_pictureBorderCheckbox_clicked()\0"
    "on_saveButton_clicked()\0on_pngButton_clicked()\0"
    "on_quitButton_clicked()\0text\0"
    "on_titleEdit_textChanged(QString)\0"
    "on_pictureBackgroundButton_clicked()\0"
    "on_actionBlack_and_White_triggered()\0"
    "on_actionNVG_triggered()\0"
    "on_actionLuminosity_triggered()\0"
    "on_actionInvert_colors_triggered()\0"
    "on_actionHorizontal_flip_triggered()\0"
    "on_actionVertical_flip_triggered()\0"
    "on_actionRotate_90_left_triggered()\0"
    "on_actionRotate_90_right_triggered()\0"
    "on_actionSet_as_background_triggered()\0"
    "on_actionDocumentation_triggered()\0"
};

const QMetaObject FotoWall::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_FotoWall,
      qt_meta_data_FotoWall, 0 }
};

const QMetaObject *FotoWall::metaObject() const
{
    return &staticMetaObject;
}

void *FotoWall::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FotoWall))
        return static_cast<void*>(const_cast< FotoWall*>(this));
    if (!strcmp(_clname, "Ui::MainWindow"))
        return static_cast< Ui::MainWindow*>(const_cast< FotoWall*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int FotoWall::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_loadButton_clicked(); break;
        case 1: on_addPictures_clicked(); break;
        case 2: on_pictureBorderCheckbox_clicked(); break;
        case 3: on_saveButton_clicked(); break;
        case 4: on_pngButton_clicked(); break;
        case 5: on_quitButton_clicked(); break;
        case 6: on_titleEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: on_pictureBackgroundButton_clicked(); break;
        case 8: on_actionBlack_and_White_triggered(); break;
        case 9: on_actionNVG_triggered(); break;
        case 10: on_actionLuminosity_triggered(); break;
        case 11: on_actionInvert_colors_triggered(); break;
        case 12: on_actionHorizontal_flip_triggered(); break;
        case 13: on_actionVertical_flip_triggered(); break;
        case 14: on_actionRotate_90_left_triggered(); break;
        case 15: on_actionRotate_90_right_triggered(); break;
        case 16: on_actionSet_as_background_triggered(); break;
        case 17: on_actionDocumentation_triggered(); break;
        }
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
