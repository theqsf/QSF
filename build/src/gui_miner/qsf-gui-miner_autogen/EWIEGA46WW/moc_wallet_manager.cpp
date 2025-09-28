/****************************************************************************
** Meta object code from reading C++ file 'wallet_manager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/gui_miner/wallet_manager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wallet_manager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_qsf__GuiWalletManager_t {
    QByteArrayData data[10];
    char stringdata0[109];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_qsf__GuiWalletManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_qsf__GuiWalletManager_t qt_meta_stringdata_qsf__GuiWalletManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "qsf::GuiWalletManager"
QT_MOC_LITERAL(1, 22, 12), // "walletOpened"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 7), // "address"
QT_MOC_LITERAL(4, 44, 12), // "walletClosed"
QT_MOC_LITERAL(5, 57, 14), // "balanceUpdated"
QT_MOC_LITERAL(6, 72, 7), // "balance"
QT_MOC_LITERAL(7, 80, 5), // "error"
QT_MOC_LITERAL(8, 86, 7), // "message"
QT_MOC_LITERAL(9, 94, 14) // "onRefreshTimer"

    },
    "qsf::GuiWalletManager\0walletOpened\0\0"
    "address\0walletClosed\0balanceUpdated\0"
    "balance\0error\0message\0onRefreshTimer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_qsf__GuiWalletManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    0,   42,    2, 0x06 /* Public */,
       5,    1,   43,    2, 0x06 /* Public */,
       7,    1,   46,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    8,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void qsf::GuiWalletManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GuiWalletManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->walletOpened((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->walletClosed(); break;
        case 2: _t->balanceUpdated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->onRefreshTimer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GuiWalletManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiWalletManager::walletOpened)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GuiWalletManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiWalletManager::walletClosed)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GuiWalletManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiWalletManager::balanceUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GuiWalletManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiWalletManager::error)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject qsf::GuiWalletManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_qsf__GuiWalletManager.data,
    qt_meta_data_qsf__GuiWalletManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *qsf::GuiWalletManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *qsf::GuiWalletManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_qsf__GuiWalletManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int qsf::GuiWalletManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void qsf::GuiWalletManager::walletOpened(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void qsf::GuiWalletManager::walletClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void qsf::GuiWalletManager::balanceUpdated(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void qsf::GuiWalletManager::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
