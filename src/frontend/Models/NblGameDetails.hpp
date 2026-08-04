#pragma once

#include <qstringview.h>
#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblGameDetails : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId CONSTANT)
    Q_PROPERTY(qint64 displayId READ displayId CONSTANT)
    Q_PROPERTY(QByteArray title READ title CONSTANT)
    Q_PROPERTY(QByteArray description READ description CONSTANT)
    Q_PROPERTY(QByteArray poster READ poster CONSTANT)
    Q_PROPERTY(QByteArray banner READ banner CONSTANT)
    Q_PROPERTY(QByteArray icon READ icon CONSTANT)
    Q_PROPERTY(QByteArray logo READ logo CONSTANT)
    Q_PROPERTY(quint64 logoPosition READ logoPosition CONSTANT)
    Q_PROPERTY(quint64 playTime READ playTime CONSTANT)
    Q_PROPERTY(quint64 playCount READ playCount CONSTANT)
    Q_PROPERTY(quint64 lastPlayed READ lastPlayed CONSTANT)
    Q_PROPERTY(quint64 added READ added CONSTANT)
    Q_PROPERTY(quint64 installationCount READ installationCount CONSTANT)
    Q_PROPERTY(quint64 releaseDate READ releaseDate CONSTANT)
    Q_PROPERTY(double rating READ rating CONSTANT)

public:

    NblGameDetails(QObject* parent = nullptr) : 
        QObject(parent),
        _mGameId(0),
        _mDisplayId(0),
        _mTitle{},
        _mDescription{},
        _mPoster{},
        _mBanner{},
        _mIcon{},
        _mLogo{},
        _mLogoPosition(0),
        _mPlayTime(0),
        _mPlayCount(0),
        _mLastPlayed(0),
        _mAdded(0),
        _mInstallationCount(0),
        _mReleaseDate(0),
        _mRating(0.0)
    {}

    NblGameDetails(
        qint64 gameId,
        qint64 displayId,
        QByteArray title,
        QByteArray description,
        QByteArray poster,
        QByteArray banner,
        QByteArray icon,
        QByteArray logo,
        quint64 logoPosition,
        quint64 playTime,
        quint64 playCount,
        quint64 lastPlayed,
        quint64 added,
        quint64 installationCount,
        quint64 releaseDate,
        f64 rating,
        QObject* parent = nullptr
    ) : 
        QObject(parent),
        _mGameId(gameId),
        _mDisplayId(displayId),
        _mTitle(title),
        _mDescription(description),
        _mPoster(poster),
        _mBanner(banner),
        _mIcon(icon),
        _mLogo(logo),
        _mLogoPosition(logoPosition),
        _mPlayTime(playTime),
        _mPlayCount(playCount),
        _mLastPlayed(lastPlayed),
        _mAdded(added),
        _mInstallationCount(installationCount),
        _mReleaseDate(releaseDate),
        _mRating(rating)
    {}

    NblGameDetails(const NblGameDetails& other) :
        QObject(other.parent()),
        _mGameId(other._mGameId),
        _mDisplayId(other._mDisplayId),
        _mTitle(other._mTitle),
        _mDescription(other._mDescription),
        _mPoster(other._mPoster),
        _mBanner(other._mBanner),
        _mIcon(other._mIcon),
        _mLogo(other._mLogo),
        _mPlayTime(other._mPlayTime),
        _mPlayCount(other._mPlayCount),
        _mLastPlayed(other._mLastPlayed),
        _mAdded(other._mAdded),
        _mInstallationCount(other._mInstallationCount),
        _mReleaseDate(other._mReleaseDate),
        _mRating(other._mRating)
    {}

    NblGameDetails& operator=(const NblGameDetails& other) {
        if(this == &other) return *this;

        setParent(other.parent());
        _mGameId = other._mGameId;
        _mDisplayId = other._mDisplayId;
        _mGameId = other._mGameId;
        _mTitle = other._mTitle;
        _mDescription = other._mDescription;
        _mPoster = other._mPoster;
        _mBanner = other._mBanner;
        _mIcon = other._mIcon;
        _mLogo = other._mLogo;
        _mPlayTime = other._mPlayTime;
        _mPlayCount = other._mPlayCount;
        _mLastPlayed = other._mLastPlayed;
        _mAdded = other._mAdded;
        _mInstallationCount = other._mInstallationCount;
        _mReleaseDate = other._mReleaseDate;
        _mRating = other._mRating;

        return *this;
    }

    qint64 gameId() const { return _mGameId; }
    qint64 displayId() const { return _mDisplayId; }
    QByteArray title() const { return _mTitle; }
    QByteArray description() const { return _mDescription; }
    QByteArray poster() const { return _mPoster; }
    QByteArray banner() const { return _mBanner; }
    QByteArray icon() const { return _mIcon; }
    QByteArray logo() const { return _mLogo; }
    quint64 logoPosition() const { return _mLogoPosition; }
    quint64 playTime() const { return _mPlayTime; }
    quint64 playCount() const { return _mPlayCount; }
    quint64 lastPlayed() const { return _mLastPlayed; }
    quint64 added() const { return _mAdded; }
    quint64 installationCount() const { return _mInstallationCount; }
    quint64 releaseDate() const { return _mReleaseDate; }
    f64 rating() const { return _mRating; }

    Q_INVOKABLE QUrl posterUrl();
    Q_INVOKABLE QUrl bannerUrl();
    Q_INVOKABLE QUrl iconUrl();
    Q_INVOKABLE QUrl logoUrl();

private:

    qint64 _mGameId;
    qint64 _mDisplayId;
    QByteArray _mTitle;
    QByteArray _mDescription;
    QByteArray _mPoster;
    QByteArray _mBanner;
    QByteArray _mIcon;
    QByteArray _mLogo;
    quint64 _mLogoPosition;
    quint64 _mPlayTime;
    quint64 _mPlayCount;
    quint64 _mLastPlayed;
    quint64 _mAdded;
    quint64 _mInstallationCount;
    quint64 _mReleaseDate;
    f64 _mRating;

};
