/****************************************************************************
** Meta object code from reading C++ file 'mining_worker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/gui_miner/mining_worker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mining_worker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_qsf__MiningWorker_t {
    QByteArrayData data[13];
    char stringdata0[159];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_qsf__MiningWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_qsf__MiningWorker_t qt_meta_stringdata_qsf__MiningWorker = {
    {
QT_MOC_LITERAL(0, 0, 17), // "qsf::MiningWorker"
QT_MOC_LITERAL(1, 18, 13), // "miningStarted"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 13), // "miningStopped"
QT_MOC_LITERAL(4, 47, 15), // "hashRateUpdated"
QT_MOC_LITERAL(5, 63, 8), // "hashRate"
QT_MOC_LITERAL(6, 72, 15), // "sharesSubmitted"
QT_MOC_LITERAL(7, 88, 6), // "shares"
QT_MOC_LITERAL(8, 95, 5), // "error"
QT_MOC_LITERAL(9, 101, 13), // "onMiningTimer"
QT_MOC_LITERAL(10, 115, 14), // "onZmqConnected"
QT_MOC_LITERAL(11, 130, 17), // "onZmqDisconnected"
QT_MOC_LITERAL(12, 148, 10) // "onZmqError"

    },
    "qsf::MiningWorker\0miningStarted\0\0"
    "miningStopped\0hashRateUpdated\0hashRate\0"
    "sharesSubmitted\0shares\0error\0onMiningTimer\0"
    "onZmqConnected\0onZmqDisconnected\0"
    "onZmqError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_qsf__MiningWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    0,   60,    2, 0x06 /* Public */,
       4,    1,   61,    2, 0x06 /* Public */,
       6,    1,   64,    2, 0x06 /* Public */,
       8,    1,   67,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   70,    2, 0x08 /* Private */,
      10,    0,   71,    2, 0x08 /* Private */,
      11,    0,   72,    2, 0x08 /* Private */,
      12,    1,   73,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    5,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::QString,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,

       0        // eod
};

void qsf::MiningWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MiningWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->miningStarted(); break;
        case 1: _t->miningStopped(); break;
        case 2: _t->hashRateUpdated((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->sharesSubmitted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onMiningTimer(); break;
        case 6: _t->onZmqConnected(); break;
        case 7: _t->onZmqDisconnected(); break;
        case 8: _t->onZmqError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MiningWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MiningWorker::miningStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MiningWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MiningWorker::miningStopped)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MiningWorker::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MiningWorker::hashRateUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MiningWorker::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MiningWorker::sharesSubmitted)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MiningWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MiningWorker::error)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject qsf::MiningWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_qsf__MiningWorker.data,
    qt_meta_data_qsf__MiningWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *qsf::MiningWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *qsf::MiningWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_qsf__MiningWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int qsf::MiningWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void qsf::MiningWorker::miningStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void qsf::MiningWorker::miningStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void qsf::MiningWorker::hashRateUpdated(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void qsf::MiningWorker::sharesSubmitted(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void qsf::MiningWorker::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
