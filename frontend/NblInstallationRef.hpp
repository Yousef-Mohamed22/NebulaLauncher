#pragma once

#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblInstallationRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int64_t installationId READ installationId CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString poster READ poster CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

public:

    NblInstallationRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mTitle{},
        _mPoster{}
    {}
    
    NblInstallationRef(i64 id, QString title, QString poster, QString icon, QObject* parent = nullptr) :
        QObject(parent),
        _mInstallationId(id),
        _mTitle(title),
        _mPoster(poster),
        _mIcon(icon)
    { }

    NblInstallationRef(const NblInstallationRef& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
        _mTitle(other._mTitle),
        _mPoster(other._mPoster),
        _mIcon(other._mIcon)
    { }

    NblInstallationRef& operator=(const NblInstallationRef& other) {
        if(&other == this) return *this;
        _mInstallationId = other._mInstallationId;
        _mTitle = other._mTitle;
        _mPoster = other._mPoster;
        _mIcon = other._mIcon;

        setParent(other.parent());
        return *this;
    }

    i64 installationId() { return _mInstallationId; }
    QString title() { return _mTitle; }
    QString poster() { return _mPoster; }
    QString icon() { return _mIcon; }

private:

    i64 _mInstallationId;
    QString _mTitle;
    QString _mPoster;
    QString _mIcon;

};

