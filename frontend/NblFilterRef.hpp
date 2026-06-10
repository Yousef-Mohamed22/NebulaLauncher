#pragma once

#include <qtmetamacros.h>
#include <Core/Types.hpp>

#include <QObject>
#include <QtQml>

class NblFilterRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 filterId READ filterId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(bool multiLine READ multiLine CONSTANT)

public:

    NblFilterRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mFilterId(0),
        _mName{},
        _mIcon{},
        _mMultiLine(false)
    {}

    NblFilterRef(i64 id, QString name, QString icon, bool multiLine, QObject* parent = nullptr) :
        QObject(parent),
        _mFilterId(id),
        _mName(name),
        _mIcon(icon),
        _mMultiLine(multiLine)
    { }

    NblFilterRef(const NblFilterRef& other) :
        QObject(other.parent()),
        _mFilterId(other._mFilterId),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mMultiLine(other._mMultiLine)
    { }

    NblFilterRef& operator=(const NblFilterRef& other) {
        if(&other == this) return *this;
        _mFilterId = other._mFilterId;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mMultiLine = other._mMultiLine;

        setParent(other.parent());
        return *this;
    }

    qint64 filterId() { return _mFilterId; }
    QString name() { return _mName; }
    QString icon() { return _mIcon; }
    bool multiLine() { return _mMultiLine; }

private:
    
    i64 _mFilterId;
    QString _mName;
    QString _mIcon;
    bool _mMultiLine;

};



