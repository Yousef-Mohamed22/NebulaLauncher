#pragma once

#include <qqmllist.h>
#include <qtmetamacros.h>
#include <Core/Types.hpp>
#include <Data/Model.hpp>

#include <QObject>
#include <QtQml>

#include <Data/Database.hpp>

class NblTabRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 tabId READ tabId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

    Q_PROPERTY(bool colorizeIcon READ colorizeIcon CONSTANT)
    Q_PROPERTY(bool hidden READ hidden CONSTANT)


public:

    explicit NblTabRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mName{},
        _mIcon{},
        _mTabId(0),
        _mFlags(0)
    {}

    NblTabRef(const NblTabRef& other) :
        _mTabId(other._mTabId),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mFlags(other._mFlags)
    { }

    virtual ~NblTabRef() = default;

    NblTabRef& operator=(const NblTabRef& other) {
        if(&other == this) return *this;
        _mTabId = other._mTabId;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mFlags = other._mFlags;
        return *this;
    }



    explicit NblTabRef(const nbl::TabMeta& meta, QObject* parent = nullptr);

    qint64 tabId() const { return _mTabId; }
    QString name() const { return _mName; }
    QString icon() const { return _mIcon; }
    bool colorizeIcon() const { return _mFlags & nbl::TabFlag::COLORIZE_ICON; }
    bool hidden() const { return _mFlags & nbl::TabFlag::HIDDEN; }

    Q_INVOKABLE QVariantList encodeFilters();

    static NblTabRef* lookup(qint64 id);

private:

    qint64 _mTabId;
    QString _mName;
    QString _mIcon;
    nbl::TabFlags _mFlags;

};
