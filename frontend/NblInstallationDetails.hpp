#pragma once

#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblInstallationDetails : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(i64 installationId READ installationId CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString poster READ poster CONSTANT)
    Q_PROPERTY(QString banner READ banner CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QString logo READ logo CONSTANT)
    Q_PROPERTY(u64 playTime READ playTime CONSTANT)
    Q_PROPERTY(u64 playCount READ playCount CONSTANT)
    Q_PROPERTY(u64 lastPlayed READ lastPlayed CONSTANT)
    Q_PROPERTY(u64 added READ added CONSTANT)
    Q_PROPERTY(u64 releaseDate READ releaseDate CONSTANT)
    Q_PROPERTY(f64 rating READ rating CONSTANT)
    Q_PROPERTY(QString gameTitle READ gameTitle CONSTANT)
    Q_PROPERTY(QString gamePoster READ gamePoster CONSTANT)

public:

    NblInstallationDetails(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mTitle{},
        _mDescription{},
        _mPoster{},
        _mBanner{},
        _mIcon{},
        _mLogo{},
        _mPlayTime(0),
        _mPlayCount(0),
        _mLastPlayed(0),
        _mAdded(0),
        _mReleaseDate(0),
        _mRating(0.0),
        _mGameTitle{},
        _mGamePoster{}
    {}

    NblInstallationDetails(
        i64 installationId,
        QString title,
        QString description,
        QString poster,
        QString banner,
        QString icon,
        QString logo,
        u64 playTime,
        u64 playCount,
        u64 lastPlayed,
        u64 added,
        u64 releaseDate,
        f64 rating,
        QString gameTitle,
        QString gamePoster,
        QObject* parent = nullptr
    ) : 
        QObject(parent),
        _mInstallationId(installationId),
        _mTitle(title),
        _mDescription(description),
        _mPoster(poster),
        _mBanner(banner),
        _mIcon(icon),
        _mLogo(logo),
        _mPlayTime(playTime),
        _mPlayCount(playCount),
        _mLastPlayed(lastPlayed),
        _mAdded(added),
        _mReleaseDate(releaseDate),
        _mRating(rating),
        _mGameTitle(gameTitle),
        _mGamePoster(gamePoster)
    {}

    NblInstallationDetails(const NblInstallationDetails& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
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
        _mReleaseDate(other._mReleaseDate),
        _mRating(other._mRating),
        _mGameTitle(other._mGameTitle),
        _mGamePoster(other._mGamePoster)
    {}

    NblInstallationDetails& operator=(const NblInstallationDetails& other) {
        if(this == &other) return *this;

        setParent(other.parent());
        _mInstallationId = other._mInstallationId;
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
        _mReleaseDate = other._mReleaseDate;
        _mRating = other._mRating;
        _mGameTitle = other._mGameTitle;
        _mGamePoster = other._mGamePoster;

        return *this;
    }

    i64 installationId() const { return _mInstallationId; }
    QString title() const { return _mTitle; }
    QString description() const { return _mDescription; }
    QString poster() const { return _mPoster; }
    QString banner() const { return _mBanner; }
    QString icon() const { return _mIcon; }
    QString logo() const { return _mLogo; }
    u64 playTime() const { return _mPlayTime; }
    u64 playCount() const { return _mPlayCount; }
    u64 lastPlayed() const { return _mLastPlayed; }
    u64 added() const { return _mAdded; }
    u64 releaseDate() const { return _mReleaseDate; }
    f64 rating() const { return _mRating; }
    QString gameTitle() const { return _mGameTitle; }
    QString gamePoster() const { return _mGamePoster; }

private:

    i64 _mInstallationId;
    QString _mTitle;
    QString _mDescription;
    QString _mPoster;
    QString _mBanner;
    QString _mIcon;
    QString _mLogo;
    u64 _mPlayTime;
    u64 _mPlayCount;
    u64 _mLastPlayed;
    u64 _mAdded;
    u64 _mReleaseDate;
    f64 _mRating;
    QString _mGameTitle;
    QString _mGamePoster;

};
