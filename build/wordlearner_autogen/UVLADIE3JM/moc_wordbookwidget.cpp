/****************************************************************************
** Meta object code from reading C++ file 'wordbookwidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/wordbookwidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wordbookwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN14WordBookWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto WordBookWidget::qt_create_metaobjectdata<qt_meta_tag_ZN14WordBookWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WordBookWidget",
        "returnToMainWindow",
        "",
        "onBtnAllClicked",
        "onBtnLearnedClicked",
        "onBtnUnlearnedClicked",
        "onSearch",
        "onBack",
        "loadAllWords",
        "searchFilter",
        "loadLearnedWords",
        "loadUnlearnedWords",
        "onScrollChanged",
        "val",
        "loadNextPage",
        "showWordFullDetail",
        "wordId",
        "clearLayout",
        "QLayout*",
        "layout",
        "toggleStar",
        "star"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'returnToMainWindow'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onBtnAllClicked'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBtnLearnedClicked'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBtnUnlearnedClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSearch'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBack'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'loadAllWords'
        QtMocHelpers::SlotData<void(const QString &)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'loadAllWords'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate | QMC::MethodCloned, QMetaType::Void),
        // Slot 'loadLearnedWords'
        QtMocHelpers::SlotData<void(const QString &)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'loadLearnedWords'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate | QMC::MethodCloned, QMetaType::Void),
        // Slot 'loadUnlearnedWords'
        QtMocHelpers::SlotData<void(const QString &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'loadUnlearnedWords'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate | QMC::MethodCloned, QMetaType::Void),
        // Slot 'onScrollChanged'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'loadNextPage'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showWordFullDetail'
        QtMocHelpers::SlotData<void(int)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'clearLayout'
        QtMocHelpers::SlotData<void(QLayout *)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 18, 19 },
        }}),
        // Slot 'toggleStar'
        QtMocHelpers::SlotData<void(int, bool)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 }, { QMetaType::Bool, 21 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WordBookWidget, qt_meta_tag_ZN14WordBookWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WordBookWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14WordBookWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14WordBookWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14WordBookWidgetE_t>.metaTypes,
    nullptr
} };

void WordBookWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WordBookWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->returnToMainWindow(); break;
        case 1: _t->onBtnAllClicked(); break;
        case 2: _t->onBtnLearnedClicked(); break;
        case 3: _t->onBtnUnlearnedClicked(); break;
        case 4: _t->onSearch(); break;
        case 5: _t->onBack(); break;
        case 6: _t->loadAllWords((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->loadAllWords(); break;
        case 8: _t->loadLearnedWords((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->loadLearnedWords(); break;
        case 10: _t->loadUnlearnedWords((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->loadUnlearnedWords(); break;
        case 12: _t->onScrollChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->loadNextPage(); break;
        case 14: _t->showWordFullDetail((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->clearLayout((*reinterpret_cast<std::add_pointer_t<QLayout*>>(_a[1]))); break;
        case 16: _t->toggleStar((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QLayout* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WordBookWidget::*)()>(_a, &WordBookWidget::returnToMainWindow, 0))
            return;
    }
}

const QMetaObject *WordBookWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WordBookWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14WordBookWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WordBookWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void WordBookWidget::returnToMainWindow()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
