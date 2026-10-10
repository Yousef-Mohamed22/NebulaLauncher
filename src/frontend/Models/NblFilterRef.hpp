#pragma once

#include <qtmetamacros.h>
#include <Core/Types.hpp>

#include <QObject>
#include <QtQml>
#include "Data/Model.hpp"

class NblFilterRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 filterId READ filterId CONSTANT)
    Q_PROPERTY(QByteArray name READ name CONSTANT)
    Q_PROPERTY(QByteArray icon READ icon CONSTANT)

    Q_PROPERTY(bool multiLine READ multiLine CONSTANT)
    Q_PROPERTY(bool showTitles READ showTitles CONSTANT)
    Q_PROPERTY(bool useIcons READ useIcons CONSTANT)
    Q_PROPERTY(bool onlyInstalled READ onlyInstalled CONSTANT)
    Q_PROPERTY(bool colorizeIcon READ colorizeIcon CONSTANT)
    Q_PROPERTY(bool showInstallations READ showInstallations CONSTANT)
    Q_PROPERTY(bool showFilterInfo READ showFilterInfo CONSTANT)

public:

    NblFilterRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mFilterId(0),
        _mName{},
        _mIcon{},
        _mFlags(0)
    {}

    NblFilterRef(i64 id, QByteArray name, QByteArray icon, nbl::FilterFlags flags, QObject* parent = nullptr) :
        QObject(parent),
        _mFilterId(id),
        _mName(name),
        _mIcon(icon),
        _mFlags(flags)
    { }

    NblFilterRef(const NblFilterRef& other) :
        QObject(other.parent()),
        _mFilterId(other._mFilterId),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mFlags(other._mFlags)
    { }

    NblFilterRef& operator=(const NblFilterRef& other) {
        if(&other == this) return *this;
        _mFilterId = other._mFilterId;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mFlags = other._mFlags;

        setParent(other.parent());
        return *this;
    }

    qint64 filterId() { return _mFilterId; }
    QByteArray name() { return _mName; }
    QByteArray icon() { return _mIcon; }

    bool multiLine() { return _mFlags & nbl::FilterFlag::MULTI_LINE; }
    bool showTitles() { return _mFlags & nbl::FilterFlag::SHOW_TITLES; }
    bool useIcons() { return _mFlags & nbl::FilterFlag::USE_ICONS; }
    bool onlyInstalled() { return _mFlags & nbl::FilterFlag::ONLY_INSTALLED; }
    bool showInstallations() { return _mFlags & nbl::FilterFlag::SHOW_INSTALLATIONS; }
    bool colorizeIcon() { return _mFlags & nbl::FilterFlag::COLORIZE_ICON; }
    bool showFilterInfo() { return _mFlags & nbl::FilterFlag::SHOW_FILTER_INFO; }

private:
    
    i64 _mFilterId;
    QByteArray _mName;
    QByteArray _mIcon;
    nbl::FilterFlags _mFlags;

};



