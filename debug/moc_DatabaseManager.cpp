/****************************************************************************
** Meta object code from reading C++ file 'DatabaseManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../DatabaseManager.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DatabaseManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN15DatabaseManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto DatabaseManager::qt_create_metaobjectdata<qt_meta_tag_ZN15DatabaseManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DatabaseManager",
        "tasksUpdated",
        "",
        "taskUpdateFailed",
        "taskId",
        "taskAdded",
        "taskRemoved",
        "notesUpdated",
        "loggedIn",
        "loggedOut",
        "groupChanged",
        "notesChanged",
        "QList<QMap<QString,QVariant>>",
        "notes",
        "taskUpdated",
        "success",
        "updateTask",
        "TaskUpdates",
        "updates",
        "removeTask"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'tasksUpdated'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'taskUpdateFailed'
        QtMocHelpers::SignalData<void(int)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'taskAdded'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'taskRemoved'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'notesUpdated'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'loggedIn'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'loggedOut'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'groupChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'notesChanged'
        QtMocHelpers::SignalData<void(const QList<QMap<QString,QVariant>> &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Signal 'taskUpdated'
        QtMocHelpers::SignalData<void(int, bool)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Bool, 15 },
        }}),
        // Signal 'taskRemoved'
        QtMocHelpers::SignalData<void(int, bool)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Bool, 15 },
        }}),
        // Method 'updateTask'
        QtMocHelpers::MethodData<bool(int, const TaskUpdates &)>(16, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 4 }, { 0x80000000 | 17, 18 },
        }}),
        // Method 'removeTask'
        QtMocHelpers::MethodData<bool(int)>(19, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 4 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DatabaseManager, qt_meta_tag_ZN15DatabaseManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DatabaseManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DatabaseManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DatabaseManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15DatabaseManagerE_t>.metaTypes,
    nullptr
} };

void DatabaseManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DatabaseManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->tasksUpdated(); break;
        case 1: _t->taskUpdateFailed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->taskAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->taskRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->notesUpdated(); break;
        case 5: _t->loggedIn(); break;
        case 6: _t->loggedOut(); break;
        case 7: _t->groupChanged(); break;
        case 8: _t->notesChanged((*reinterpret_cast< std::add_pointer_t<QList<QMap<QString,QVariant>>>>(_a[1]))); break;
        case 9: _t->taskUpdated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 10: _t->taskRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 11: { bool _r = _t->updateTask((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<TaskUpdates>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 12: { bool _r = _t->removeTask((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)()>(_a, &DatabaseManager::tasksUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(int )>(_a, &DatabaseManager::taskUpdateFailed, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(int )>(_a, &DatabaseManager::taskAdded, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(int )>(_a, &DatabaseManager::taskRemoved, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)()>(_a, &DatabaseManager::notesUpdated, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)()>(_a, &DatabaseManager::loggedIn, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)()>(_a, &DatabaseManager::loggedOut, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)()>(_a, &DatabaseManager::groupChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(const QList<QMap<QString,QVariant>> & )>(_a, &DatabaseManager::notesChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(int , bool )>(_a, &DatabaseManager::taskUpdated, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(int , bool )>(_a, &DatabaseManager::taskRemoved, 10))
            return;
    }
}

const QMetaObject *DatabaseManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DatabaseManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DatabaseManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DatabaseManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void DatabaseManager::tasksUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DatabaseManager::taskUpdateFailed(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void DatabaseManager::taskAdded(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void DatabaseManager::taskRemoved(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void DatabaseManager::notesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DatabaseManager::loggedIn()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void DatabaseManager::loggedOut()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void DatabaseManager::groupChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void DatabaseManager::notesChanged(const QList<QMap<QString,QVariant>> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void DatabaseManager::taskUpdated(int _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void DatabaseManager::taskRemoved(int _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2);
}
QT_WARNING_POP
