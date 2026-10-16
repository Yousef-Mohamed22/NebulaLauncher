/**
 * \file InstallationFunctions.cpp
 * \author jwall64
 * \brief Partial implementation of ApplicationContext.hpp, containing functions related to managing games
 */


#include "ApplicationContext.hpp"
#include "Data/Model.hpp"
#include "Field/FieldRef.hpp"

#include <Core/Result.hpp>
#include <Data/Database.hpp>
#include <Data/MediaManager.hpp>
#include <Plugin/AchievementProvider.hpp>

nbl::frontend::GameDetails* ApplicationContext::getGameDetails(qint64 gameId) {

    if(!databaseConnection()->isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot get game details, database connection is not valid");
        return nullptr;
    }

    nbl::frontend::GameDetails* out = nullptr;
    if(nbl::getGameDetails(databaseConnection(), gameId, [&](const nbl::GameDetails& details) {

        out = new nbl::frontend::GameDetails(
            details.game.id,
            nbl::frontend::fromNbl(details.game.title),
            nbl::frontend::fromNbl(details.game.description),
            details.game.poster,
            details.game.banner,
            details.game.icon,
            details.game.logo,
            static_cast<u64>(details.game.logoPosition),
            details.totalPlayTime,
            details.totalPlayCount,
            details.lastPlayed,
            details.game.added,
            details.installationCount,
            details.game.releaseDate,
            details.game.rating
        );

        return nbl::Result::SUCCESS;
    }).error()) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to get installation details for game with ID ", gameId);
        return nullptr;
    }

    return out;
}

qint64 ApplicationContext::addGame(QVariantMap params) {

    QString bannerUrl = params.find("banner")->toString();
    QString posterUrl = params.find("poster")->toString();
    QString iconUrl = params.find("icon")->toString();
    QString logoUrl = params.find("logo")->toString();

    i64 banner = 0;
    i64 poster = 0;
    i64 icon = 0;
    i64 logo = 0;

    if(!bannerUrl.isEmpty()) NBL_CHECK(pushImage(bannerUrl, nbl::ImageClass::BANNER, &banner));
    if(!posterUrl.isEmpty()) NBL_CHECK(pushImage(posterUrl, nbl::ImageClass::POSTER, &poster));
    if(!iconUrl.isEmpty())   NBL_CHECK(pushImage(iconUrl, nbl::ImageClass::ICON, &icon));
    if(!logoUrl.isEmpty())   NBL_CHECK(pushImage(logoUrl, nbl::ImageClass::LOGO, &logo));
    nbl::LogoPosition position = static_cast<nbl::LogoPosition>(params.find("logoPosition")->toUInt());

    QByteArray title = params.find("title")->toString().toUtf8();
    QByteArray description = params.find("description")->toString().toUtf8();
    u64 releaseDate = params.find("releaseDate")->toULongLong();
    f64 rating = params.find("rating")->toDouble();

    nbl::GameMeta gameMeta = {
        .id = 0,
        .title = nbl::StringView(title.constData(), title.length()),
        .description = nbl::StringView(description.constData(), description.length()),
        .banner = banner,
        .poster = poster,
        .icon = icon,
        .logo = logo,
        .logoPosition = position,
        .releaseDate = releaseDate,
        .rating = rating
    };

    NBL_CHECK(databaseConnection()->beginTransaction());
    NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::addGame(databaseConnection(), gameMeta, &gameMeta.id));

    for(QVariant qs : params.find("tags")->toList()) {
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), tagGame(gameMeta.id, qs.toLongLong()));
    }

    QVariantList externalIds = params.find("externalIds")->toList();
    QHash<QByteArray, QByteArray> idMap{};
    idMap.reserve(externalIds.length());

    for(QVariant qs : externalIds) {
        QByteArray name = qs.toMap().find("platform")->toString().toUtf8();
        QByteArray id = qs.toMap().find("id")->toString().toUtf8();
        nbl::StringView nblName(name.data(), name.length());
        nbl::StringView nblId(id.data(), id.length());
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::setGameExternalId(databaseConnection(), gameMeta.id, nblName, nblId));

        idMap[name] = id;
    }

    QVariantMap achievementProviderFields;
    if(params.contains("achievementProviders")) achievementProviderFields = params.find("achievementProviders")->toMap();
    for(const nbl::AchievementProvider* ap : _mContext.registry<nbl::AchievementProvider>()) {

        QString qId = QString::fromUtf8(ap->id.data, ap->id.length);

        nbl::AchievementProviderStatus status = nbl::AchievementProviderStatus::NOT_ADDED;
        nbl::Span<nbl::FieldValue> fieldValues{};

        auto provIt = achievementProviderFields.find(qId);
        if(provIt != achievementProviderFields.end()) {

            QVariantMap providerInfo = provIt->toMap();

            status = static_cast<nbl::AchievementProviderStatus>(providerInfo.find("status")->toUInt());
            if(status == nbl::AchievementProviderStatus::ADDED_BY_USER) {
                QVariantMap fields = providerInfo.find("fields")->toMap();

                nbl::ArenaFrame frame(nbl::Arena::threadLocal());
                nbl::Field dummy {
                    .type = nbl::FieldType::STRUCT,
                    .structData = { .fields = ap->fields }
                };

                fieldValues = nbl::frontend::FieldRef(&dummy).toNbl(fields, &frame).compositeValue;

                nbl::StringBuilder builder(&frame);
                NBL_CHECK(nbl::encodeFieldValues(&_mContext, ap->fields, fieldValues, builder, {}));

                nbl::StringView fieldsJson;
                NBL_CHECK(builder.buildNullTerminated(&fieldsJson));

                NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::upsertAchievementState(databaseConnection(), gameMeta.id, {
                    .gameId = gameMeta.id,
                    .provider = ap->id,
                    .fields = fieldsJson,
                    .status = status
                }));
            }
        }

        if(status == nbl::AchievementProviderStatus::REMOVED_BY_USER
        || status == nbl::AchievementProviderStatus::REMOVED_BY_PROVIDER) {
            continue;
        }

        nbl::AchievementHelper helper(databaseConnection(), ap->id, gameMeta.id);

        QByteArray requested = QByteArray(ap->requestedExternalId.data, ap->requestedExternalId.length);
        QByteArray externalId = idMap.value(requested);

        nbl::Result res = ap->updateAchievementState(
                &_mContext, 
                gameMeta,
                status, 
                fieldValues,
                helper, 
                nbl::StringView(externalId.data(), externalId.length()));
        if(res.error()) {

            _mContext.pushNotification(
                    nbl::NotificationType::WARNING, 
                    "An Error Occurred", 
                    "An error occurred while an achievement provider was attempting to update a newly-added game!"
            );
            nbl::log(nbl::LogLevel::WARN, "An error occurred while an achievement provider was attempting to update a newly-added game! ", res.value);
            continue;
        }
    }

    NBL_CHECK(databaseConnection()->commitTransaction());

    return gameMeta.id;
}


int ApplicationContext::updateGame(QVariantMap params) {

    bool ok = true;
    i64 gId = params.find("id")->toLongLong(&ok);
    if(!ok) {
        return nbl::Result::INVALID_ARGUMENT;
    }

    QString bannerUrl = params.find("banner")->toString();
    QString posterUrl = params.find("poster")->toString();
    QString iconUrl = params.find("icon")->toString();
    QString logoUrl = params.find("logo")->toString();

    i64 banner = 0;
    i64 poster = 0;
    i64 icon = 0;
    i64 logo = 0;

    if(!bannerUrl.isEmpty()) NBL_CHECK(pushImage(bannerUrl, nbl::ImageClass::BANNER, &banner));
    if(!posterUrl.isEmpty()) NBL_CHECK(pushImage(posterUrl, nbl::ImageClass::POSTER, &poster));
    if(!iconUrl.isEmpty())   NBL_CHECK(pushImage(iconUrl, nbl::ImageClass::ICON, &icon));
    if(!logoUrl.isEmpty())   NBL_CHECK(pushImage(logoUrl, nbl::ImageClass::LOGO, &logo));

    nbl::LogoPosition position = static_cast<nbl::LogoPosition>(params.find("logoPosition")->toUInt());


    QByteArray title = params.find("title")->toString().toUtf8();
    QByteArray description = params.find("description")->toString().toUtf8();
    u64 releaseDate = params.find("releaseDate")->toULongLong();
    f64 rating = params.find("rating")->toDouble();

    nbl::GameMeta gameMeta = {
        .id = gId,
        .title = nbl::StringView(title.constData(), title.length()),
        .description = nbl::StringView(description.constData(), description.length()),
        .banner = banner,
        .poster = poster,
        .icon = icon,
        .logo = logo,
        .logoPosition = position,
        .releaseDate = releaseDate,
        .rating = rating
    };

    NBL_CHECK(databaseConnection()->beginTransaction());
    NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::updateGame(databaseConnection(), gameMeta));

    for(QVariant qs : params.find("tags")->toList()) {
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), tagGame(gId, qs.toLongLong()));
    }
    for(QVariant qs : params.find("removedTags")->toList()) {
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), untagGame(gId, qs.toLongLong()));
    }

    nbl::clearGameExternalIds(databaseConnection(), gId);
    QVariantList externalIds = params.find("externalIds")->toList();
    QHash<QByteArray, QByteArray> idMap{};
    idMap.reserve(externalIds.length());

    for(QVariant qs : externalIds) {
        QByteArray name = qs.toMap().find("platform")->toString().toUtf8();
        QByteArray id = qs.toMap().find("id")->toString().toUtf8();
        nbl::StringView nblName(name.data(), name.length());
        nbl::StringView nblId(id.data(), id.length());
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::setGameExternalId(databaseConnection(), gId, nblName, nblId));

        idMap[name] = id;
    }

    QVariantMap achievementProviderFields = params.find("achievementProviders")->toMap();
    for(const nbl::AchievementProvider* ap : _mContext.registry<nbl::AchievementProvider>()) {

        QString qId = QString::fromUtf8(ap->id.data, ap->id.length);
        QVariantMap providerInfo = achievementProviderFields.find(qId)->toMap();

        nbl::AchievementProviderStatus status = static_cast<nbl::AchievementProviderStatus>(providerInfo.find("status")->toUInt());
        nbl::AchievementProviderStatus originalStatus = static_cast<nbl::AchievementProviderStatus>(providerInfo.find("originalStatus")->toUInt());

        if(status == nbl::AchievementProviderStatus::REMOVED_BY_USER
        || status == nbl::AchievementProviderStatus::REMOVED_BY_PROVIDER) {

            if(originalStatus != status) {
                NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::upsertAchievementState(databaseConnection(), gameMeta.id, {
                    .gameId = gameMeta.id,
                    .provider = ap->id,
                    .fields = {},
                    .status = status
                }));
                NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::deleteAchievementSetsForProvider(databaseConnection(), gameMeta.id, ap->id));
            }
            continue;
        }


        QVariantMap fields = providerInfo.find("fields")->toMap();

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::Field dummy {
            .type = nbl::FieldType::STRUCT,
            .structData = { .fields = ap->fields }
        };

        nbl::FieldValue value = nbl::frontend::FieldRef(&dummy).toNbl(fields, &frame);

        nbl::StringBuilder builder(&frame);
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::encodeFieldValues(&_mContext, ap->fields, value.compositeValue, builder, {}));

        nbl::StringView fieldsJson;
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), builder.buildNullTerminated(&fieldsJson));

        if(status == nbl::AchievementProviderStatus::ADDED_BY_USER && status != originalStatus) {
            NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::upsertAchievementState(databaseConnection(), gameMeta.id, {
                .gameId = gameMeta.id,
                .provider = ap->id,
                .fields = fieldsJson,
                .status = status
            }));
        }

        nbl::AchievementHelper helper(databaseConnection(), ap->id, gameMeta.id);

        QByteArray requested = QByteArray(ap->requestedExternalId.data, ap->requestedExternalId.length);
        QByteArray externalId = idMap.value(requested);

        nbl::Result res = ap->updateAchievementState(
                &_mContext, 
                gameMeta,
                status, 
                value.compositeValue,
                helper, 
                nbl::StringView(externalId.data(), externalId.length()));
        if(res.error()) {

            _mContext.pushNotification(
                    nbl::NotificationType::WARNING, 
                    "An Error Occurred", 
                    "An error occurred while an achievement provider was attempting to update an existing game!"
            );
            nbl::log(nbl::LogLevel::WARN, "An error occurred while an achievement provider was attempting to update an existing game! ", res.value);
            continue;
        }
    }

    for(QVariant qs : params.find("hiddenSets")->toList()) {
        i64 id = qs.toMap().find("id")->toLongLong();
        bool hidden = qs.toMap().find("hidden")->toBool();

        NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::updateAchivementSetHidden(databaseConnection(), id, hidden));
    }

    NBL_CHECK(databaseConnection()->commitTransaction());

    return 0;
}

int ApplicationContext::deleteGame(qint64 gameId) {
    return nbl::removeGame(databaseConnection(), gameId);
}


qint64 ApplicationContext::getRandomGameId() {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(databaseConnection())) {
        if(databaseConnection()->prepare("SELECT gId FROM game WHERE EXISTS (SELECT 1 FROM installation WHERE installation.gId = game.gId) ORDER BY RANDOM() LIMIT 1;", &_stmt).error()) {
            return 0;
        }
    }

    qint64 out = 0;
    if(databaseConnection()->execute(&_stmt, [&](i32 num,nbl::DataValue* values,const char**,i64) -> nbl::Result {
        NBL_ASSERT(num == 1);
        out = values[0].intValue;
        return nbl::Result::SUCCESS;
    }).error()) {
        return 0;
    }

    return out;
}


QVariantMap ApplicationContext::gameExternalIds(qint64 gameId) {
    QVariantMap out = {};
    nbl::enumerateGameExternalIds(databaseConnection(), gameId, [&](i64 gameId, nbl::StringView platformName, nbl::StringView platformId) -> nbl::Result {
        out[QString(nbl::frontend::fromNbl(platformName))] = QString(nbl::frontend::fromNbl(platformId));
        return nbl::Result::SUCCESS;
    });

    return out;
}
