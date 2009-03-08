/****************************************************************************
** Meta object code from reading C++ file 'BackgroundChooser.h'
**
** Created: Fri Nov 14 22:44:33 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "BackgroundChooser.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BackgroundChooser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BackgroundChooser[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   18,   18,   18, 0x0a,
      69,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_BackgroundChooser[] = {
    "BackgroundChooser\0\0signalImageChoosed(QString)\0"
    "slotOkButtonClicked()\0slotAddOtherImage()\0"
};

const QMetaObject BackgroundChooser::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_BackgroundChooser,
      qt_meta_data_BackgroundChooser, 0 }
};

const QMetaObject *BackgroundChooser::metaObject() const
{
    return &staticMetaObject;
}

void *BackgroundChooser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BackgroundChooser))
        return static_cast<void*>(const_cast< BackgroundChooser*>(this));
    if (!strcmp(_clname, "Ui_BackgroundChooser"))
        return static_cast< Ui_BackgroundChooser*>(const_cast< BackgroundChooser*>(this));
    return QDialog::qt_metacast(_clname);
}

int BackgroundChooser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: signalImageChoosed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: slotOkButtonClicked(); break;
        case 2: slotAddOtherImage(); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void BackgroundChooser::signalImageChoosed(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
