#pragma once

#include <qstringview.h>
#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblInstallationRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int64_t installationId READ installationId CONSTANT)
    Q_PROPERTY(QByteArray title READ title CONSTANT)
    Q_PROPERTY(QByteArray gameTitle READ title CONSTANT)
    Q_PROPERTY(QByteArray poster READ poster CONSTANT)
    Q_PROPERTY(QByteArray icon READ icon CONSTANT)
    Q_PROPERTY(quint64 lastPlayed READ lastPlayed CONSTANT)

    Q_PROPERTY(QByteArray resolvedTitle READ resolvedTitle CONSTANT)

public:

    NblInstallationRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mTitle{},
        _mGameTitle{},
        _mPoster{},
        _mLastPlayed(0)
    {}
    
    NblInstallationRef(i64 id, QByteArray title, QByteArray gameTitle, QByteArray poster, QByteArray icon, u64 lastPlayed, QObject* parent = nullptr) :
        QObject(parent),
        _mInstallationId(id),
        _mTitle(title),
        _mGameTitle(gameTitle),
        _mPoster(poster),
        _mIcon(icon),
        _mLastPlayed(lastPlayed)
    { }

    NblInstallationRef(const NblInstallationRef& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
        _mTitle(other._mTitle),
        _mGameTitle(other._mGameTitle),
        _mPoster(other._mPoster),
        _mIcon(other._mIcon),
        _mLastPlayed(other._mLastPlayed)
    { }

    NblInstallationRef& operator=(const NblInstallationRef& other) {
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
    QByteArray poster() const { return _mPoster; }
    QByteArray icon() const { return _mIcon; }
    u64 lastPlayed() const { return _mLastPlayed; }

    QByteArray resolvedTitle() const {
        return _mTitle.length() == 0 ? _mGameTitle : _mTitle;
    }

private:

    i64 _mInstallationId;
    QByteArray _mTitle;
    QByteArray _mGameTitle;
    QByteArray _mPoster;
    QByteArray _mIcon;
    u64 _mLastPlayed;

};

