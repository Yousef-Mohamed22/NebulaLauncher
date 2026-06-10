#pragma once

#include <Core/Types.hpp>

#include <QObject>
#include <QtQml>

class NblTabRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(qint64 tabId READ tabId CONSTANT)

public:

    explicit NblTabRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mName{},
        _mIcon{},
        _mTabId(0)
    {}

    virtual ~NblTabRef() = default;

    NblTabRef(qint64 id, QString name, QString icon, QObject* parent = nullptr) :
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
    qint64 tabId() { return _mTabId; }

private:

    QString _mName;
    QString _mIcon;
    qint64 _mTabId;

};
