#pragma once

#include <qstringview.h>
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
    Q_PROPERTY(QByteArray type READ type CONSTANT)
    Q_PROPERTY(QByteArray command READ command CONSTANT)
    Q_PROPERTY(QByteArray title READ title CONSTANT)
    Q_PROPERTY(QByteArray description READ description CONSTANT)
    Q_PROPERTY(QByteArray poster READ poster CONSTANT)
    Q_PROPERTY(QByteArray banner READ banner CONSTANT)
    Q_PROPERTY(QByteArray icon READ icon CONSTANT)
    Q_PROPERTY(QByteArray logo READ logo CONSTANT)
    Q_PROPERTY(uint64_t logoPosition READ logoPosition CONSTANT)
    Q_PROPERTY(uint64_t playTime READ playTime CONSTANT)
    Q_PROPERTY(uint64_t playCount READ playCount CONSTANT)
    Q_PROPERTY(uint64_t lastPlayed READ lastPlayed CONSTANT)
    Q_PROPERTY(uint64_t added READ added CONSTANT)
    Q_PROPERTY(uint64_t releaseDate READ releaseDate CONSTANT)
    Q_PROPERTY(uint64_t steamId READ steamId CONSTANT)
    Q_PROPERTY(uint64_t igdbId READ igdbId CONSTANT)
    Q_PROPERTY(double rating READ rating CONSTANT)
    Q_PROPERTY(QByteArray gameTitle READ gameTitle CONSTANT)
    Q_PROPERTY(QByteArray gameDescription READ gameDescription CONSTANT)

public:

    NblInstallationDetails(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
        _mDisplayId(0),
        _mGameId(0),
        _mType{},
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
        QByteArray type,
        QByteArray command,
        QByteArray title,
        QByteArray description,
        QByteArray poster,
        QByteArray banner,
        QByteArray icon,
        QByteArray logo,
        u64 logoPosition,
        u64 playTime,
        u64 playCount,
        u64 lastPlayed,
        u64 added,
        u64 releaseDate,
        f64 rating,
        u64 steamId,
        u64 igdbId,
        QByteArray gameTitle,
        QByteArray gameDescription,
        QObject* parent = nullptr
    ) : 
        QObject(parent),
        _mInstallationId(installationId),
        _mDisplayId(displayId),
        _mGameId(gameId),
        _mType(type),
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
        _mType(other._mType),
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
        _mType = other._mType;
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
    QByteArray type() const { return _mType; }
    QByteArray command() const { return _mCommand; }
    QByteArray title() const { return _mTitle; }
    QByteArray description() const { return _mDescription; }
    QByteArray poster() const { return _mPoster; }
    QByteArray banner() const { return _mBanner; }
    QByteArray icon() const { return _mIcon; }
    QByteArray logo() const { return _mLogo; }
    u64 logoPosition() const { return _mLogoPosition; }
    u64 playTime() const { return _mPlayTime; }
    u64 playCount() const { return _mPlayCount; }
    u64 lastPlayed() const { return _mLastPlayed; }
    u64 added() const { return _mAdded; }
    u64 releaseDate() const { return _mReleaseDate; }
    f64 rating() const { return _mRating; }
    u64 steamId() const { return _mSteamId; }
    u64 igdbId() const { return _mIgdbId; }
    QByteArray gameTitle() const { return _mGameTitle; }
    QByteArray gameDescription() const { return _mGameDescription; }

    Q_INVOKABLE QByteArray posterPath();
    Q_INVOKABLE QByteArray bannerPath();
    Q_INVOKABLE QByteArray iconPath();
    Q_INVOKABLE QByteArray logoPath();

private:

    i64 _mInstallationId;
    i64 _mDisplayId;
    i64 _mGameId;
    QByteArray _mType;
    QByteArray _mCommand;
    QByteArray _mTitle;
    QByteArray _mDescription;
    QByteArray _mPoster;
    QByteArray _mBanner;
    QByteArray _mIcon;
    QByteArray _mLogo;
    u64 _mLogoPosition;
    u64 _mPlayTime;
    u64 _mPlayCount;
    u64 _mLastPlayed;
    u64 _mAdded;
    u64 _mReleaseDate;
    f64 _mRating;
    u64 _mSteamId;
    u64 _mIgdbId;
    QByteArray _mGameTitle;
    QByteArray _mGameDescription;

};
