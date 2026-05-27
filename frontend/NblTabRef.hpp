#pragma once

#include <Core/Types.hpp>

#include <QObject>
#include <QtQml>

#include "NblFilterRef.hpp"

class NblTabRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(i64 tabId READ tabId CONSTANT)

    Q_PROPERTY(QQmlListProperty<NblFilterRef> filters READ filters NOTIFY filtersUpdated)

public:

    explicit NblTabRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mName{},
        _mIcon{},
        _mTabId(0)
    {}

    virtual ~NblTabRef() = default;

    NblTabRef(i64 id, QString name, QString icon, QObject* parent = nullptr) :
        QObject(parent),
        _mName(name),
        _mIcon(icon),
        _mTabId(id)
    { }

    NblTabRef(const NblTabRef& other) :
        QObject(other.parent()),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mTabId(other._mTabId)
    { }

    NblTabRef& operator=(const NblTabRef& other) {
        if(&other == this) return *this;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mTabId = other._mTabId;

        setParent(other.parent());
        return *this;
    }

    QString name() { return _mName; }
    QString icon() { return _mIcon; }
    i64 tabId() { return _mTabId; }

    QQmlListProperty<NblFilterRef> filters() {
        return QQmlListProperty<NblFilterRef>(
            this,
            nullptr,
            [](QQmlListProperty<NblFilterRef>* ref) -> qsizetype {
                NblTabRef* tab = static_cast<NblTabRef*>(ref->object);
                return tab->_mFilters.count();
            },
            [](QQmlListProperty<NblFilterRef>* ref, qsizetype at) -> NblFilterRef* {
                NblTabRef* tab = static_cast<NblTabRef*>(ref->object);
                if(at > tab->_mFilters.count()) return nullptr;
                return tab->_mFilters.data() + at;
            }
        );
    }

    Q_INVOKABLE i32 loadFilters();

signals:

    void filtersUpdated();

private:

    QString _mName;
    QString _mIcon;
    i64 _mTabId;
    QList<NblFilterRef> _mFilters;

};
