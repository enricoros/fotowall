/****************************************************************************
** Meta object code from reading C++ file 'FWFoto.h'
**
** Created: Fri Nov 14 22:44:31 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FWFoto.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FWFoto.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FWFoto[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x05,
      24,    7,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      56,   43,    7,    7, 0x08,
      76,   43,    7,    7, 0x08,
     108,   43,    7,    7, 0x08,
     128,    7,    7,    7, 0x08,
     144,    7,    7,    7, 0x08,
     164,    7,    7,    7, 0x08,
     182,    7,    7,    7, 0x08,
     198,    7,    7,    7, 0x0a,
     210,    7,    7,    7, 0x0a,
     238,  232,    7,    7, 0x0a,
     258,    7,    7,    7, 0x0a,
     277,    7,    7,    7, 0x0a,
     289,    7,    7,    7, 0x0a,
     307,  301,    7,    7, 0x0a,
     323,    7,    7,    7, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_FWFoto[] = {
    "FWFoto\0\0deletePressed()\0imageSelected(int)\0"
    "controlPoint\0slotResize(QPointF)\0"
    "slotResizeProportional(QPointF)\0"
    "slotRotate(QPointF)\0slotResetSize()\0"
    "slotResetRotation()\0slotResizeEnded()\0"
    "slotSaveImage()\0slotToNVG()\0"
    "slotToBlackAndWhite()\0value\0"
    "slotLuminosity(int)\0slotInvertColors()\0"
    "slotFlipH()\0slotFlipV()\0angle\0"
    "slotRotate(int)\0slotRotate()\0"
};

const QMetaObject FWFoto::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FWFoto,
      qt_meta_data_FWFoto, 0 }
};

const QMetaObject *FWFoto::metaObject() const
{
    return &staticMetaObject;
}

void *FWFoto::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FWFoto))
        return static_cast<void*>(const_cast< FWFoto*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< FWFoto*>(this));
    return QObject::qt_metacast(_clname);
}

int FWFoto::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: deletePressed(); break;
        case 1: imageSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: slotResize((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 3: slotResizeProportional((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 4: slotRotate((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 5: slotResetSize(); break;
        case 6: slotResetRotation(); break;
        case 7: slotResizeEnded(); break;
        case 8: slotSaveImage(); break;
        case 9: slotToNVG(); break;
        case 10: slotToBlackAndWhite(); break;
        case 11: slotLuminosity((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: slotInvertColors(); break;
        case 13: slotFlipH(); break;
        case 14: slotFlipV(); break;
        case 15: slotRotate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: slotRotate(); break;
        }
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void FWFoto::deletePressed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void FWFoto::imageSelected(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_FWButton[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      16,   10,    9,    9, 0x05,
      34,    9,    9,    9, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_FWButton[] = {
    "FWButton\0\0point\0dragging(QPointF)\0"
    "reset()\0"
};

const QMetaObject FWButton::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FWButton,
      qt_meta_data_FWButton, 0 }
};

const QMetaObject *FWButton::metaObject() const
{
    return &staticMetaObject;
}

void *FWButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FWButton))
        return static_cast<void*>(const_cast< FWButton*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< FWButton*>(this));
    return QObject::qt_metacast(_clname);
}

int FWButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: dragging((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 1: reset(); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void FWButton::dragging(const QPointF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FWButton::reset()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
