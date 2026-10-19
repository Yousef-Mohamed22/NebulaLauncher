/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <QObject>
#include <QtQml>
#include <QColor>

#include <Core/Types.hpp>
#include <Core/Optional.hpp>

namespace nbl::frontend {

class GameDetails : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(qint64 poster READ poster CONSTANT)
    Q_PROPERTY(qint64 banner READ banner CONSTANT)
    Q_PROPERTY(qint64 icon READ icon CONSTANT)
    Q_PROPERTY(qint64 logo READ logo CONSTANT)
    Q_PROPERTY(quint64 logoPosition READ logoPosition CONSTANT)
    Q_PROPERTY(quint64 playTime READ playTime CONSTANT)
    Q_PROPERTY(quint64 playCount READ playCount CONSTANT)
    Q_PROPERTY(quint64 lastPlayed READ lastPlayed CONSTANT)
    Q_PROPERTY(quint64 added READ added CONSTANT)
    Q_PROPERTY(quint64 installationCount READ installationCount CONSTANT)
    Q_PROPERTY(quint64 releaseDate READ releaseDate CONSTANT)
    Q_PROPERTY(double rating READ rating CONSTANT)
    Q_PROPERTY(QVariant colorOverride READ colorOverride CONSTANT)

public:

    GameDetails(QObject* parent = nullptr) : 
        QObject(parent),
        _mGameId(0),
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
        _mRating(0.0),
        _mColorOverride{}
    {}

    GameDetails(
        qint64 gameId,
        QString title,
        QString description,
        qint64 poster,
        qint64 banner,
        qint64 icon,
        qint64 logo,
        quint64 logoPosition,
        quint64 playTime,
        quint64 playCount,
        quint64 lastPlayed,
        quint64 added,
        quint64 installationCount,
        quint64 releaseDate,
        f64 rating,
        Optional<QColor> colorOverride,
        QObject* parent = nullptr
    ) : 
        QObject(parent),
        _mGameId(gameId),
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
        _mRating(rating),
        _mColorOverride(colorOverride)
    {}

    GameDetails(const GameDetails& other) :
        QObject(other.parent()),
        _mGameId(other._mGameId),
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
        _mRating(other._mRating),
        _mColorOverride(other._mColorOverride)
    {}

    GameDetails& operator=(const GameDetails& other) {
        if(this == &other) return *this;

        setParent(other.parent());
        _mGameId = other._mGameId;
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
        _mColorOverride = other._mColorOverride;

        return *this;
    }

    qint64 gameId() const { return _mGameId; }
    QString title() const { return _mTitle; }
    QString description() const { return _mDescription; }
    qint64 poster() const { return _mPoster; }
    qint64 banner() const { return _mBanner; }
    qint64 icon() const { return _mIcon; }
    qint64 logo() const { return _mLogo; }
    quint64 logoPosition() const { return _mLogoPosition; }
    quint64 playTime() const { return _mPlayTime; }
    quint64 playCount() const { return _mPlayCount; }
    quint64 lastPlayed() const { return _mLastPlayed; }
    quint64 added() const { return _mAdded; }
    quint64 installationCount() const { return _mInstallationCount; }
    quint64 releaseDate() const { return _mReleaseDate; }
    f64 rating() const { return _mRating; }
    QVariant colorOverride() const { return _mColorOverride.hasValue() ? *_mColorOverride : QVariant(); }

    Q_INVOKABLE QUrl posterUrl();
    Q_INVOKABLE QUrl bannerUrl();
    Q_INVOKABLE QUrl iconUrl();
    Q_INVOKABLE QUrl logoUrl();

private:

    qint64 _mGameId;
    QString _mTitle;
    QString _mDescription;
    qint64 _mPoster;
    qint64 _mBanner;
    qint64 _mIcon;
    qint64 _mLogo;
    quint64 _mLogoPosition;
    quint64 _mPlayTime;
    quint64 _mPlayCount;
    quint64 _mLastPlayed;
    quint64 _mAdded;
    quint64 _mInstallationCount;
    quint64 _mReleaseDate;
    f64 _mRating;
    Optional<QColor> _mColorOverride;

};

}
