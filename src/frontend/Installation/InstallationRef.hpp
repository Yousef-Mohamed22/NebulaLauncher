#pragma once

#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

namespace nbl::frontend {

class InstallationRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int64_t installationId READ installationId CONSTANT)
    Q_PROPERTY(QByteArray title READ title CONSTANT)
    Q_PROPERTY(QByteArray gameTitle READ title CONSTANT)
    Q_PROPERTY(qint64 poster READ poster CONSTANT)
    Q_PROPERTY(qint64 icon READ icon CONSTANT)
    Q_PROPERTY(quint64 lastPlayed READ lastPlayed CONSTANT)

    Q_PROPERTY(QByteArray resolvedTitle READ resolvedTitle CONSTANT)

public:

    InstallationRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mTitle{},
        _mGameTitle{},
        _mPoster{},
        _mLastPlayed(0)
    {}
    
    InstallationRef(i64 id, QByteArray title, QByteArray gameTitle, qint64 poster, qint64 icon, u64 lastPlayed, QObject* parent = nullptr) :
        QObject(parent),
        _mInstallationId(id),
        _mTitle(title),
        _mGameTitle(gameTitle),
        _mPoster(poster),
        _mIcon(icon),
        _mLastPlayed(lastPlayed)
    { }

    InstallationRef(const InstallationRef& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
        _mTitle(other._mTitle),
        _mGameTitle(other._mGameTitle),
        _mPoster(other._mPoster),
        _mIcon(other._mIcon),
        _mLastPlayed(other._mLastPlayed)
    { }

    InstallationRef& operator=(const InstallationRef& other) {
        if(&other == this) return *this;
        _mInstallationId = other._mInstallationId;
        _mTitle = other._mTitle;
        _mGameTitle = other._mGameTitle;
        _mPoster = other._mPoster;
        _mIcon = other._mIcon;
        _mLastPlayed = other._mLastPlayed;

        setParent(other.parent());
        return *this;
    }

    i64 installationId() const { return _mInstallationId; }
    QByteArray title() const { return _mTitle; }
    QByteArray gameTitle() const { return _mGameTitle; }
    qint64 poster() const { return _mPoster; }
    qint64 icon() const { return _mIcon; }
    u64 lastPlayed() const { return _mLastPlayed; }

    QByteArray resolvedTitle() const {
        return _mTitle.length() == 0 ? _mGameTitle : _mTitle;
    }

private:

    i64 _mInstallationId;
    QByteArray _mTitle;
    QByteArray _mGameTitle;
    qint64 _mPoster;
    qint64 _mIcon;
    u64 _mLastPlayed;

};

}
