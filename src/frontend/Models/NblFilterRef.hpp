#pragma once

#include <qtmetamacros.h>
#include <Core/Types.hpp>

#include <QObject>
#include <QtQml>

class NblFilterRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 filterId READ filterId CONSTANT)
    Q_PROPERTY(QByteArray name READ name CONSTANT)
    Q_PROPERTY(QByteArray icon READ icon CONSTANT)
    Q_PROPERTY(bool multiLine READ multiLine CONSTANT)
    Q_PROPERTY(bool showTitles READ showTitles CONSTANT)
    Q_PROPERTY(bool useIcons READ useIcons CONSTANT)

public:

    NblFilterRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mFilterId(0),
        _mName{},
        _mIcon{},
        _mMultiLine(false),
        _mShowTitles(false),
        _mUseIcons(false)
    {}

    NblFilterRef(i64 id, QByteArray name, QByteArray icon, bool multiLine, bool showTitles, bool useIcons, QObject* parent = nullptr) :
        QObject(parent),
        _mFilterId(id),
        _mName(name),
        _mIcon(icon),
        _mMultiLine(multiLine),
        _mShowTitles(showTitles),
        _mUseIcons(useIcons)
    { }

    NblFilterRef(const NblFilterRef& other) :
        QObject(other.parent()),
        _mFilterId(other._mFilterId),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mMultiLine(other._mMultiLine),
        _mShowTitles(other._mShowTitles),
        _mUseIcons(other._mUseIcons)
    { }

    NblFilterRef& operator=(const NblFilterRef& other) {
        if(&other == this) return *this;
        _mFilterId = other._mFilterId;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mMultiLine = other._mMultiLine;
        _mShowTitles = other._mShowTitles;
        _mUseIcons = other._mUseIcons;

        setParent(other.parent());
        return *this;
    }

    qint64 filterId() { return _mFilterId; }
    QByteArray name() { return _mName; }
    QByteArray icon() { return _mIcon; }
    bool multiLine() { return _mMultiLine; }
    bool showTitles() { return _mShowTitles; }
    bool useIcons() { return _mUseIcons; }

private:
    
    i64 _mFilterId;
    QByteArray _mName;
    QByteArray _mIcon;
    bool _mMultiLine;
    bool _mShowTitles;
    bool _mUseIcons;

};



