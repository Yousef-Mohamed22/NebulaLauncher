#pragma once

#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblInstallationDetails : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int64_t installationId READ installationId CONSTANT)
    Q_PROPERTY(int64_t displayId READ displayId CONSTANT)
    Q_PROPERTY(int64_t gameId READ gameId CONSTANT)
    Q_PROPERTY(QString command READ command CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString poster READ poster CONSTANT)
    Q_PROPERTY(QString banner READ banner CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QString logo READ logo CONSTANT)
    Q_PROPERTY(uint64_t logoPosition READ logoPosition CONSTANT)
    Q_PROPERTY(uint64_t playTime READ playTime CONSTANT)
    Q_PROPERTY(uint64_t playCount READ playCount CONSTANT)
    Q_PROPERTY(uint64_t lastPlayed READ lastPlayed CONSTANT)
    Q_PROPERTY(uint64_t added READ added CONSTANT)
    Q_PROPERTY(uint64_t releaseDate READ releaseDate CONSTANT)
    Q_PROPERTY(uint64_t steamId READ steamId CONSTANT)
    Q_PROPERTY(uint64_t igdbId READ igdbId CONSTANT)
    Q_PROPERTY(double rating READ rating CONSTANT)
    Q_PROPERTY(QString gameTitle READ gameTitle CONSTANT)
    Q_PROPERTY(QString gameDescription READ gameDescription CONSTANT)

public:

    NblInstallationDetails(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mDisplayId(0),
        _mGameId(0),
        _mCommand{},
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
        _mReleaseDate(0),
        _mRating(0.0),
        _mGameTitle{},
        _mGameDescription{}
    {}

    NblInstallationDetails(
        i64 installationId,
        i64 displayId,
        i64 gameId,
        QString command,
        QString title,
        QString description,
        QString poster,
        QString banner,
        QString icon,
        QString logo,
        u64 logoPosition,
        u64 playTime,
        u64 playCount,
        u64 lastPlayed,
        u64 added,
        u64 releaseDate,
        f64 rating,
        u64 steamId,
        u64 igdbId,
        QString gameTitle,
        QString gameDescription,
        QObject* parent = nullptr
    ) : 
        QObject(parent),
        _mInstallationId(installationId),
        _mDisplayId(displayId),
        _mGameId(gameId),
        _mCommand(command),
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
        _mReleaseDate(releaseDate),
        _mRating(rating),
        _mSteamId(steamId),
        _mIgdbId(igdbId),
        _mGameTitle(gameTitle),
        _mGameDescription(gameDescription)
    {}

    NblInstallationDetails(const NblInstallationDetails& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
        _mDisplayId(other._mDisplayId),
        _mGameId(other._mGameId),
        _mCommand(other._mCommand),
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
        _mSteamId(other._mSteamId),
        _mIgdbId(other._mIgdbId),
        _mGameTitle(other._mGameTitle),
        _mGameDescription(other._mGameDescription)
    {}

    NblInstallationDetails& operator=(const NblInstallationDetails& other) {
        if(this == &other) return *this;

        setParent(other.parent());
        _mInstallationId = other._mInstallationId;
        _mDisplayId = other._mDisplayId;
        _mGameId = other._mGameId;
        _mCommand = other._mCommand;
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
        _mSteamId = other._mSteamId;
        _mIgdbId = other._mIgdbId;
        _mGameTitle = other._mGameTitle;
        _mGameDescription = other._mGameDescription;

        return *this;
    }

    i64 installationId() const { return _mInstallationId; }
    i64 displayId() const { return _mDisplayId; }
    i64 gameId() const { return _mGameId; }
    QString command() const { return _mCommand; }
    QString title() const { return _mTitle; }
    QString description() const { return _mDescription; }
    QString poster() const { return _mPoster; }
    QString banner() const { return _mBanner; }
    QString icon() const { return _mIcon; }
    QString logo() const { return _mLogo; }
    u64 logoPosition() const { return _mLogoPosition; }
    u64 playTime() const { return _mPlayTime; }
    u64 playCount() const { return _mPlayCount; }
    u64 lastPlayed() const { return _mLastPlayed; }
    u64 added() const { return _mAdded; }
    u64 releaseDate() const { return _mReleaseDate; }
    f64 rating() const { return _mRating; }
    u64 steamId() const { return _mSteamId; }
    u64 igdbId() const { return _mIgdbId; }
    QString gameTitle() const { return _mGameTitle; }
    QString gameDescription() const { return _mGameDescription; }

    Q_INVOKABLE QString posterPath();
    Q_INVOKABLE QString bannerPath();
    Q_INVOKABLE QString iconPath();
    Q_INVOKABLE QString logoPath();

private:

    i64 _mInstallationId;
    i64 _mDisplayId;
    i64 _mGameId;
    QString _mCommand;
    QString _mTitle;
    QString _mDescription;
    QString _mPoster;
    QString _mBanner;
    QString _mIcon;
    QString _mLogo;
    u64 _mLogoPosition;
    u64 _mPlayTime;
    u64 _mPlayCount;
    u64 _mLastPlayed;
    u64 _mAdded;
    u64 _mReleaseDate;
    f64 _mRating;
    u64 _mSteamId;
    u64 _mIgdbId;
    QString _mGameTitle;
    QString _mGameDescription;

};
