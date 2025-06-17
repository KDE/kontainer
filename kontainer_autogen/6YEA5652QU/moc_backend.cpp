/****************************************************************************
** Meta object code from reading C++ file 'backend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/backend.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>

#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'backend.h' doesn't include <QObject>."
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
namespace
{
struct qt_meta_tag_ZN7BackendE_t {
};
} // unnamed namespace

template<>
constexpr inline auto Backend::qt_create_metaobjectdata<qt_meta_tag_ZN7BackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData{"Backend",
                                                 "assembleFinished",
                                                 "",
                                                 "result",
                                                 "assembleContainer",
                                                 "iniFile",
                                                 "installDebPackage",
                                                 "terminal",
                                                 "containerName",
                                                 "filePath",
                                                 "installRpmPackage",
                                                 "installArchPackage"};

    QtMocHelpers::UintData qt_methods{
        // Signal 'assembleFinished'
        QtMocHelpers::SignalData<void(const QString &)>(1,
                                                        2,
                                                        QMC::AccessPublic,
                                                        QMetaType::Void,
                                                        {{
                                                            {QMetaType::QString, 3},
                                                        }}),
        // Slot 'assembleContainer'
        QtMocHelpers::SlotData<void(const QString &)>(4,
                                                      2,
                                                      QMC::AccessPublic,
                                                      QMetaType::Void,
                                                      {{
                                                          {QMetaType::QString, 5},
                                                      }}),
        // Slot 'installDebPackage'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(6,
                                                                                        2,
                                                                                        QMC::AccessPublic,
                                                                                        QMetaType::Void,
                                                                                        {{
                                                                                            {QMetaType::QString, 7},
                                                                                            {QMetaType::QString, 8},
                                                                                            {QMetaType::QString, 9},
                                                                                        }}),
        // Slot 'installRpmPackage'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(10,
                                                                                        2,
                                                                                        QMC::AccessPublic,
                                                                                        QMetaType::Void,
                                                                                        {{
                                                                                            {QMetaType::QString, 7},
                                                                                            {QMetaType::QString, 8},
                                                                                            {QMetaType::QString, 9},
                                                                                        }}),
        // Slot 'installArchPackage'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(11,
                                                                                        2,
                                                                                        QMC::AccessPublic,
                                                                                        QMetaType::Void,
                                                                                        {{
                                                                                            {QMetaType::QString, 7},
                                                                                            {QMetaType::QString, 8},
                                                                                            {QMetaType::QString, 9},
                                                                                        }}),
    };
    QtMocHelpers::UintData qt_properties{};
    QtMocHelpers::UintData qt_enums{};
    return QtMocHelpers::metaObjectData<Backend, qt_meta_tag_ZN7BackendE_t>(QMC::MetaObjectFlag{}, qt_stringData, qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Backend::staticMetaObject = {{QMetaObject::SuperData::link<QObject::staticMetaObject>(),
                                                            qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.stringdata,
                                                            qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.data,
                                                            qt_static_metacall,
                                                            nullptr,
                                                            qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7BackendE_t>.metaTypes,
                                                            nullptr}};

void Backend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Backend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0:
            _t->assembleFinished((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            break;
        case 1:
            _t->assembleContainer((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            break;
        case 2:
            _t->installDebPackage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),
                                  (*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),
                                  (*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])));
            break;
        case 3:
            _t->installRpmPackage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),
                                  (*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),
                                  (*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])));
            break;
        case 4:
            _t->installArchPackage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),
                                   (*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),
                                   (*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])));
            break;
        default:;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString &)>(_a, &Backend::assembleFinished, 0))
            return;
    }
}

const QMetaObject *Backend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Backend::qt_metacast(const char *_clname)
{
    if (!_clname)
        return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.strings))
        return static_cast<void *>(this);
    return QObject::qt_metacast(_clname);
}

int Backend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void Backend::assembleFinished(const QString &_t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
