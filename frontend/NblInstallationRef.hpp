#pragma once

#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblInstallationRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(i64 installationId READ installationId CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString poster READ poster CONSTANT)

public:

    NblInstallationRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mTitle{},
        _mPoster{}
    {}
    
    NblInstallationRef(i64 id, QString title, QString poster, QObject* parent = nullptr) :
        QObject(parent),
        _mInstallationId(id),
        _mTitle(title),
        _mPoster(poster)
    { }

    NblInstallationRef(const NblInstallationRef& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
        _mTitle(other._mTitle),
        _mPoster(other._mPoster)
    { }

    NblInstallationRef& operator=(const NblInstallationRef& other) {
        if(&other == this) return *this;
        _mInstallationId = other._mInstallationId;
        _mTitle = other._mTitle;
        _mPoster = other._mPoster;

        setParent(other.parent());
        return *this;
    }

    i64 installationId() { return _mInstallationId; }
    QString title() { return _mTitle; }
    QString poster() { return _mPoster; }

private:

    i64 _mInstallationId;
    QString _mTitle;
    QString _mPoster;

};

