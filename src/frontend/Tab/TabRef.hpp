#pragma once

#include <Core/Types.hpp>
#include <Data/Model.hpp>
#include <Data/Database.hpp>

#include <QObject>
#include <QtQml>

namespace nbl::frontend {

class TabRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 tabId READ tabId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

    Q_PROPERTY(bool iconIsImage READ iconIsImage CONSTANT)
    Q_PROPERTY(bool colorizeIcon READ colorizeIcon CONSTANT)
    Q_PROPERTY(bool hidden READ hidden CONSTANT)

public:

    explicit TabRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mName{},
        _mIcon{},
        _mTabId(0),
        _mFlags(0)
    {}

    TabRef(const TabRef& other) :
        _mTabId(other._mTabId),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mFlags(other._mFlags)
    { }

    virtual ~TabRef() = default;

    TabRef& operator=(const TabRef& other) {
        if(&other == this) return *this;
        _mTabId = other._mTabId;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mFlags = other._mFlags;
        return *this;
    }



    explicit TabRef(const nbl::TabMeta& meta, QObject* parent = nullptr);

    qint64 tabId() const { return _mTabId; }
    QString name() const { return _mName; }
    QString icon() const;
    bool iconIsImage() const { return _mFlags & nbl::TabFlag::IMAGE_ICON; }
    bool colorizeIcon() const { return _mFlags & nbl::TabFlag::COLORIZE_ICON; }
    bool hidden() const { return _mFlags & nbl::TabFlag::HIDDEN; }

    Q_INVOKABLE QVariantList encodeFilters();

    static TabRef* lookup(qint64 id);

private:

    qint64 _mTabId;
    QString _mName;
    InlineIcon _mIcon;
    nbl::TabFlags _mFlags;

};

}
