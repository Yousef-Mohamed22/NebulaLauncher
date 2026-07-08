#pragma once

#include <Core/Types.hpp>

#include <QObject>
#include <QtQml>

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
        _mColorizeIcon(false),
        _mHidden(false)
    {}

    virtual ~NblTabRef() = default;

    NblTabRef(qint64 id, QString name, QString icon, bool colorizeIcon, bool hidden, QObject* parent = nullptr) :
        QObject(parent),
        _mName(name),
        _mIcon(icon),
        _mTabId(id),
        _mColorizeIcon(colorizeIcon),
        _mHidden(hidden)
    { }

    NblTabRef(const NblTabRef& other) :
        QObject(other.parent()),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mTabId(other._mTabId),
        _mColorizeIcon(other._mColorizeIcon),
        _mHidden(other._mHidden)
    { }

    NblTabRef& operator=(const NblTabRef& other) {
        if(&other == this) return *this;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mTabId = other._mTabId;
        _mColorizeIcon = other._mColorizeIcon;
        _mHidden = other._mHidden;

        setParent(other.parent());
        return *this;
    }

    qint64 tabId() { return _mTabId; }
    QString name() { return _mName; }
    QString icon() { return _mIcon; }
    bool colorizeIcon() { return _mColorizeIcon; }
    bool hidden() { return _mHidden; }

private:

    qint64 _mTabId;
    QString _mName;
    QString _mIcon;
    bool _mColorizeIcon;
    bool _mHidden;

};
