#pragma once

#include <qstringview.h>
#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Core/Types.hpp>

class NblInstallationDetails : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 installationId READ installationId CONSTANT)
    Q_PROPERTY(qint64 gameId READ gameId CONSTANT)
    Q_PROPERTY(QByteArray type READ type CONSTANT)
    Q_PROPERTY(QByteArray command READ command CONSTANT)
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
    Q_PROPERTY(quint64 releaseDate READ releaseDate CONSTANT)
    Q_PROPERTY(double rating READ rating CONSTANT)
    Q_PROPERTY(QByteArray gameTitle READ gameTitle CONSTANT)
    Q_PROPERTY(QByteArray gameDescription READ gameDescription CONSTANT)
    Q_PROPERTY(QByteArray gamePoster READ gamePoster CONSTANT)
    Q_PROPERTY(QByteArray gameBanner READ gameBanner CONSTANT)
    Q_PROPERTY(QByteArray gameIcon READ gameIcon CONSTANT)
    Q_PROPERTY(QByteArray gameLogo READ gameLogo CONSTANT)
    Q_PROPERTY(quint64 gameLogoPosition READ gameLogoPosition CONSTANT)

    Q_PROPERTY(QByteArray resolvedTitle READ resolvedTitle CONSTANT)
    Q_PROPERTY(QUrl resolvedPosterUrl READ resolvedPosterUrl CONSTANT)
    Q_PROPERTY(QUrl resolvedBannerUrl READ resolvedBannerUrl CONSTANT)
    Q_PROPERTY(QUrl resolvedIconUrl READ resolvedIconUrl CONSTANT)
    Q_PROPERTY(QUrl resolvedLogoUrl READ resolvedLogoUrl CONSTANT)
    Q_PROPERTY(quint64 resolvedLogoPosition READ resolvedLogoPosition CONSTANT)

public:

    NblInstallationDetails(QObject* parent = nullptr) : 
        QObject(parent),
        _mInstallationId(0),
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
        _mGameDescription{},
        _mGamePoster{},
        _mGameBanner{},
        _mGameIcon{},
        _mGameLogo{},
        _mGameLogoPosition(0)
    {}

    NblInstallationDetails(
        qint64 installationId,
        qint64 gameId,
        QByteArray type,
        QByteArray command,
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
        quint64 releaseDate,
        f64 rating,
        QByteArray gameTitle,
        QByteArray gameDescription,
        QByteArray gamePoster,
        QByteArray gameBanner,
        QByteArray gameIcon,
        QByteArray gameLogo,
        quint64 gameLogoPosition,
        QObject* parent = nullptr
    ) : 
        QObject(parent),
        _mInstallationId(installationId),
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
        _mGameTitle(gameTitle),
        _mGameDescription(gameDescription),
        _mGamePoster(gamePoster),
        _mGameBanner(gameBanner),
        _mGameIcon(gameIcon),
        _mGameLogo(gameLogo),
        _mGameLogoPosition(gameLogoPosition)
    {}

    NblInstallationDetails(const NblInstallationDetails& other) :
        QObject(other.parent()),
        _mInstallationId(other._mInstallationId),
        _mGameId(other._mGameId),
        _mType(other._mType),
        _mCommand(other._mCommand),
        _mTitle(other._mTitle),
        _mDescription(other._mDescription),
        _mPoster(other._mPoster),
        _mBanner(other._mBanner),
        _mIcon(other._mIcon),
        _mLogo(other._mLogo),
        _mLogoPosition(other._mLogoPosition),
        _mPlayTime(other._mPlayTime),
        _mPlayCount(other._mPlayCount),
        _mLastPlayed(other._mLastPlayed),
        _mAdded(other._mAdded),
        _mReleaseDate(other._mReleaseDate),
        _mRating(other._mRating),
        _mGameTitle(other._mGameTitle),
        _mGameDescription(other._mGameDescription),
        _mGamePoster(other._mGamePoster),
        _mGameBanner(other._mGameBanner),
        _mGameIcon(other._mGameIcon),
        _mGameLogo(other._mGameLogo),
        _mGameLogoPosition(other._mGameLogoPosition)
    {}

    NblInstallationDetails& operator=(const NblInstallationDetails& other) {
        if(this == &other) return *this;

        setParent(other.parent());
        _mInstallationId = other._mInstallationId;
        _mGameId = other._mGameId;
        _mType = other._mType;
        _mCommand = other._mCommand;
        _mTitle = other._mTitle;
        _mDescription = other._mDescription;
        _mPoster = other._mPoster;
        _mBanner = other._mBanner;
        _mIcon = other._mIcon;
        _mLogo = other._mLogo;
        _mLogoPosition = other._mLogoPosition;
        _mPlayTime = other._mPlayTime;
        _mPlayCount = other._mPlayCount;
        _mLastPlayed = other._mLastPlayed;
        _mAdded = other._mAdded;
        _mReleaseDate = other._mReleaseDate;
        _mRating = other._mRating;
        _mGameTitle = other._mGameTitle;
        _mGameDescription = other._mGameDescription;
        _mGamePoster = other._mGamePoster;
        _mGameBanner = other._mGameBanner;
        _mGameIcon = other._mGameIcon;
        _mGameLogo = other._mGameLogo;
        _mGameLogoPosition = other._mGameLogoPosition;

        return *this;
    }

    qint64 installationId() const { return _mInstallationId; }
    qint64 gameId() const { return _mGameId; }
    QByteArray type() const { return _mType; }
    QByteArray command() const { return _mCommand; }
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
    quint64 releaseDate() const { return _mReleaseDate; }
    f64 rating() const { return _mRating; }

    QByteArray gameTitle() const { return _mGameTitle; }
    QByteArray gameDescription() const { return _mGameDescription; }
    QByteArray gamePoster() const { return _mGamePoster; }
    QByteArray gameBanner() const { return _mGameBanner; }
    QByteArray gameIcon() const { return _mGameIcon; }
    QByteArray gameLogo() const { return _mGameLogo; }
    quint64 gameLogoPosition() const { return _mGameLogoPosition; }

    QByteArray resolvedTitle() const {
        return _mTitle.length() == 0 ? _mGameTitle : _mTitle;
    }
    QUrl resolvedPosterUrl() const {
        return _mediaUrl(_mPoster.length() == 0 ? _mGamePoster : _mPoster, "poster");
    }
    QUrl resolvedBannerUrl() const {
        return _mediaUrl(_mBanner.length() == 0 ? _mGameBanner : _mBanner, "banner");
    }
    QUrl resolvedIconUrl() const {
        return _mediaUrl(_mIcon.length() == 0 ? _mGameIcon : _mIcon, "icon");
    }
    QUrl resolvedLogoUrl() const {
        return _mediaUrl(_mLogo.length() == 0 ? _mGameLogo : _mLogo, "logo");
    }
    quint64 resolvedLogoPosition() const {
        return _mLogoPosition == 4 ? _mGameLogoPosition : _mLogoPosition;
    }

private:

    QUrl _mediaUrl(QByteArray rel, QByteArray repo) const;

    qint64 _mInstallationId;
    qint64 _mGameId;
    QByteArray _mType;
    QByteArray _mCommand;
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
    quint64 _mReleaseDate;
    f64 _mRating;
    QByteArray _mGameTitle;
    QByteArray _mGameDescription;
    QByteArray _mGamePoster;
    QByteArray _mGameBanner;
    QByteArray _mGameIcon;
    QByteArray _mGameLogo;
    quint64 _mGameLogoPosition;

};
