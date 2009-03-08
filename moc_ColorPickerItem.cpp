/****************************************************************************
** Meta object code from reading C++ file 'ColorPickerItem.h'
**
** Created: Fri Nov 14 22:44:30 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ColorPickerItem.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ColorPickerItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ColorPickerItem[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      23,   17,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      49,   44,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ColorPickerItem[] = {
    "ColorPickerItem\0\0color\0colorChanged(QColor)\0"
    "step\0slotAnimateScale(int)\0"
};

const QMetaObject ColorPickerItem::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ColorPickerItem,
      qt_meta_data_ColorPickerItem, 0 }
};

const QMetaObject *ColorPickerItem::metaObject() const
{
    return &staticMetaObject;
}

void *ColorPickerItem::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ColorPickerItem))
        return static_cast<void*>(const_cast< ColorPickerItem*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< ColorPickerItem*>(this));
    return QObject::qt_metacast(_clname);
}

int ColorPickerItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: colorChanged((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 1: slotAnimateScale((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ColorPickerItem::colorChanged(const QColor & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
