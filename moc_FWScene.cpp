/****************************************************************************
** Meta object code from reading C++ file 'FWScene.h'
**
** Created: Fri Nov 14 22:44:32 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FWScene.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FWScene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FWScene[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x08,
      26,    8,    8,    8, 0x08,
      50,    8,    8,    8, 0x08,
      73,    8,    8,    8, 0x08,
      96,    8,    8,    8, 0x08,
     119,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FWScene[] = {
    "FWScene\0\0slotDeleteFoto()\0"
    "slotTitleColorChanged()\0slotForeColorChanged()\0"
    "slotGradColorChanged()\0slotImageSelected(int)\0"
    "slotCallChangeImageBackground(QString)\0"
};

const QMetaObject FWScene::staticMetaObject = {
    { &QGraphicsScene::staticMetaObject, qt_meta_stringdata_FWScene,
      qt_meta_data_FWScene, 0 }
};

const QMetaObject *FWScene::metaObject() const
{
    return &staticMetaObject;
}

void *FWScene::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FWScene))
        return static_cast<void*>(const_cast< FWScene*>(this));
    return QGraphicsScene::qt_metacast(_clname);
}

int FWScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slotDeleteFoto(); break;
        case 1: slotTitleColorChanged(); break;
        case 2: slotForeColorChanged(); break;
        case 3: slotGradColorChanged(); break;
        case 4: slotImageSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: slotCallChangeImageBackground((*reinterpret_cast< QString(*)>(_a[1]))); break;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
