/**
 * \file InstallationFunctions.cpp
 * \author jwall64
 * \brief Partial implementation of ApplicationContext.hpp, containing functions related to managing installations
 */

#include "ApplicationContext.hpp"


namespace {


nbl::Result _applyInstallationCommand(nbl::PluginContext* context, nbl::InstallationMeta meta, const nbl::InstallationType* type, QVariantMap fields, QVariantList externalIds) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::Field dummy {
        .type = nbl::FieldType::STRUCT,
        .structData = { .fields = type->fields }
    };

    nbl::FieldValue value = nbl::frontend::FieldRef(&dummy).toNbl(fields, &frame);

    bool applyGamePlaceholders = false;
    for(usize i = 0 ; i < type->fields.length ; i++) {
        const nbl::Field& field = type->fields[i];
        if(field.type == nbl::FieldType::STRING) {
            applyGamePlaceholders = applyGamePlaceholders || (field.flags & nbl::FieldFlag::APPLY_GAME_PLACEHOLDERS);
        }
    }

    nbl::Span<nbl::Placeholder> placeholders{};
    if(applyGamePlaceholders) {

        usize globalPlaceholders = 2 + externalIds.length();
        NBL_CHECK(frame.allocate(&placeholders.data, globalPlaceholders));

        nbl::StringView gIdStr;
        nbl::StringView iIdStr;
        NBL_CHECK(nbl::StringBuilder(&frame).append(meta.gameId).buildNullTerminated(&gIdStr));
        NBL_CHECK(nbl::StringBuilder(&frame).append(meta.id).buildNullTerminated(&gIdStr));

        placeholders[placeholders.length++] = {
            .key = "gameId",
            .value = gIdStr
        };
        placeholders[placeholders.length++] = {
            .key = "installationId",
            .value = gIdStr
        };

        for(QVariant v : externalIds) {
            QByteArray name = v.toMap().find("platform")->toString().toUtf8();
            QByteArray id = v.toMap().find("id")->toString().toUtf8();

            nbl::StringView nblName(name.data(), name.length());
            nbl::StringView nblId(id.data(), id.length());

            NBL_CHECK(nbl::StringBuilder(&frame).append("external_id_").append(nblName).buildNullTerminated(&nblName));
            NBL_CHECK(nbl::StringBuilder(&frame).append(nblId).buildNullTerminated(&nblId));

            placeholders[placeholders.length++] = {
                .key = nblName,
                .value = nblId
            };
        }
    }

    nbl::StringBuilder builder(&frame);
    NBL_CHECK(nbl::encodeFieldValues(context, type->fields, value.compositeValue, builder, placeholders));

    nbl::StringView cmd;
    NBL_CHECK(builder.buildNullTerminated(&cmd));

    meta.command = cmd;
    return nbl::updateInstallation(context->databaseConnection(), meta);
}


}


nbl::frontend::InstallationDetails* ApplicationContext::getInstallationDetails(qint64 installationId) {

    if(!databaseConnection()->isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot get installation details, database connection is not valid");
        return nullptr;
    }

    nbl::frontend::InstallationDetails* out;

    if(nbl::getInstallationDetails(databaseConnection(), installationId, [&](const nbl::InstallationDetails& details) -> nbl::Result {

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());

        const nbl::InstallationType* type = _mContext.get<nbl::InstallationType>(details.installation.type);
        QVariant qtFields{};
        if(type != nullptr) {
            nbl::Span<nbl::Field> fields = type->fields;

            nbl::frontend::FieldList* list = new nbl::frontend::FieldList(fields);
            nbl::Span<nbl::FieldValue> values;

            nbl::JsonElement value = {};
            NBL_CHECK(nbl::parseJson(nbl::StringReader(details.installation.command), &frame, &value));
            NBL_CHECK(nbl::decodeFieldValues(&_mContext, type->fields, value.object, &values, &frame));

            nbl::Field dummy {
                .type = nbl::FieldType::STRUCT,
                .structData = { .fields = type->fields }
            };

            qtFields = nbl::frontend::FieldRef(&dummy).fromNbl({ .compositeValue = values });
        }

        out = new nbl::frontend::InstallationDetails(
            details.installation.id,
            details.game.id,
            fromNbl(details.installation.type),
            qtFields.toMap(),
            fromNbl(details.installation.title),
            fromNbl(details.installation.description),
            fromNbl(details.installation.poster),
            fromNbl(details.installation.banner),
            fromNbl(details.installation.icon),
            fromNbl(details.installation.logo),
            static_cast<u64>(details.installation.logoPosition),
            details.installation.playTime,
            details.installation.playCount,
            details.installation.lastPlayed,
            details.installation.added,
            details.game.releaseDate,
            details.game.rating,
            fromNbl(details.game.title),
            fromNbl(details.game.description),
            fromNbl(details.game.poster),
            fromNbl(details.game.banner),
            fromNbl(details.game.icon),
            fromNbl(details.game.logo),
            static_cast<u64>(details.game.logoPosition)
        );

        return nbl::Result::SUCCESS;
    }).error()) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to get installation details for installation with ID ", installationId);
        return nullptr;
    }

    return out;

}

InstallationAddResult ApplicationContext::addInstallation(const QVariantMap& params) {

    bool ok;
    i64 gId = params.find("game")->toLongLong(&ok);
    if(!ok || gId == 0) return nbl::Result(nbl::Result::INVALID_ARGUMENT);

    NBL_CHECK(databaseConnection()->beginTransaction());

    QString bannerUrl = params.find("banner")->toString();
    QString posterUrl = params.find("poster")->toString();
    QString iconUrl = params.find("icon")->toString();
    QString logoUrl = params.find("logo")->toString();

    if(!bannerUrl.isEmpty()) NBL_CHECK(_pushImage(bannerUrl, "banner", &bannerUrl));
    if(!posterUrl.isEmpty()) NBL_CHECK(_pushImage(posterUrl, "poster", &posterUrl));
    if(!iconUrl.isEmpty())   NBL_CHECK(_pushImage(iconUrl, "icon", &iconUrl));
    if(!logoUrl.isEmpty())   NBL_CHECK(_pushImage(logoUrl, "logo", &logoUrl));
    nbl::LogoPosition position = static_cast<nbl::LogoPosition>(params.find("logoPosition")->toUInt());

    QByteArray banner = bannerUrl.toUtf8();
    QByteArray poster = posterUrl.toUtf8();
    QByteArray icon = iconUrl.toUtf8();
    QByteArray logo = logoUrl.toUtf8();


    // Add a new installation
    QByteArray type = params.find("type")->toString().toUtf8();
    nbl::InstallationMeta meta = {
        .id = 0,
        .gameId = gId,
        .deviceId = _mDeviceId,
        .type = nbl::StringView(type.data(), type.length()),
        .command = "",
    };

    QByteArray installationTitle = params.find("title")->toString().toUtf8();
    QByteArray installationDesc = params.find("description")->toString().toUtf8();

    meta.title = nbl::StringView(installationTitle.data(), installationTitle.length());
    meta.description = nbl::StringView(installationDesc.data(), installationDesc.length());
    meta.logoPosition = nbl::LogoPosition::INHERIT;

    if(gId != 0) {
        meta.banner = nbl::StringView(banner.data(), banner.length());
        meta.poster = nbl::StringView(poster.data(), poster.length());
        meta.icon = nbl::StringView(icon.data(), icon.length());
        meta.logo = nbl::StringView(logo.data(), logo.length());
        meta.logoPosition = position;
    }

    NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::addInstallation(databaseConnection(), meta, &meta.id));

    for(QVariant tag : params.find("addedInstallationTags")->toList()) {
        tagInstallation(meta.id, tag.toLongLong());
    }
    for(QVariant tag : params.find("removedInstallationTags")->toList()) {
        untagInstallation(meta.id, tag.toLongLong());
    }
    for(QVariant tag : params.find("addedGameTags")->toList()) {
        tagGame(gId, tag.toLongLong());
    }
    for(QVariant tag : params.find("removedGameTags")->toList()) {
        untagGame(gId, tag.toLongLong());
    }

    QVariantList externalIds{};
    if(gId == 0) {
        externalIds = params.find("externalIds")->toList();
        for(QVariant qs : externalIds) {
            QByteArray name = qs.toMap().find("platform")->toString().toUtf8();
            QByteArray id = qs.toMap().find("id")->toString().toUtf8();
            nbl::StringView nblName(name.data(), name.length());
            nbl::StringView nblId(id.data(), id.length());
            NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::setGameExternalId(databaseConnection(), gId, nblName, nblId));
        }
    } else {
        nbl::enumerateGameExternalIds(databaseConnection(), gId, [&](i64 gameId, nbl::StringView platformName, nbl::StringView platformId) -> nbl::Result {
            QVariantMap out{};
            out["platform"] = QString(fromNbl(platformName));
            out["id"] = QString(fromNbl(platformId));
            externalIds.append(out);
            return nbl::Result::SUCCESS;
        });
    }


    QVariantMap fields = params.find("fields")->toMap();
    const nbl::InstallationType* it = _mContext.get<nbl::InstallationType>(nbl::StringView(type.data(), type.length()));

    NBL_CHECK_OR_ROLLBACK(databaseConnection(), _applyInstallationCommand(&_mContext, meta, it, fields, externalIds));

    NBL_CHECK_OR_ROLLBACK(databaseConnection(), databaseConnection()->commitTransaction());
    return InstallationAddResult(
        true,
        gId == 0,
        gId,
        meta.id
    );
}


int ApplicationContext::updateInstallation(qint64 installationId, const QVariantMap& params) {

    QString bannerUrl = params.find("banner")->toString();
    QString posterUrl = params.find("poster")->toString();
    QString iconUrl = params.find("icon")->toString();
    QString logoUrl = params.find("logo")->toString();

    if(!bannerUrl.isEmpty()) NBL_CHECK(_pushImage(bannerUrl, "banner", &bannerUrl));
    if(!posterUrl.isEmpty()) NBL_CHECK(_pushImage(posterUrl, "poster", &posterUrl));
    if(!iconUrl.isEmpty())   NBL_CHECK(_pushImage(iconUrl, "icon", &iconUrl));
    if(!logoUrl.isEmpty())   NBL_CHECK(_pushImage(logoUrl, "logo", &logoUrl));
    nbl::LogoPosition position = static_cast<nbl::LogoPosition>(params.find("logoPosition")->toUInt());

    QByteArray banner = bannerUrl.toUtf8();
    QByteArray poster = posterUrl.toUtf8();
    QByteArray icon = iconUrl.toUtf8();
    QByteArray logo = logoUrl.toUtf8();

    bool okay = false;
    i64 gId = params.find("game")->toLongLong(&okay);
    if(!okay) return -1;

    QByteArray type = params.find("type")->toString().toUtf8();
    QByteArray installationTitle = params.find("title")->toString().toUtf8();
    QByteArray installationDesc = params.find("description")->toString().toUtf8();

    nbl::InstallationMeta meta = {
        .id = installationId,
        .gameId = gId,
        .deviceId = _mDeviceId,
        .title = nbl::StringView(installationTitle.data(), installationTitle.length()),
        .description = nbl::StringView(installationDesc.data(), installationDesc.length()),
        .banner = nbl::StringView(banner.data(), banner.length()),
        .poster = nbl::StringView(poster.data(), poster.length()),
        .icon = nbl::StringView(icon.data(), icon.length()),
        .logo = nbl::StringView(logo.data(), logo.length()),
        .logoPosition = position,
        .type = nbl::StringView(type.data(), type.length()),
        .command = ""
    };

    QVariantList externalIds{};
    NBL_CHECK(databaseConnection()->beginTransaction());
    NBL_CHECK_OR_ROLLBACK(databaseConnection(), nbl::enumerateGameExternalIds(databaseConnection(), meta.gameId, [&](i64 gameId, nbl::StringView platformName, nbl::StringView platformId) -> nbl::Result {
        QVariantMap out{};
        out["platform"] = QString(fromNbl(platformName));
        out["id"] = QString(fromNbl(platformId));
        externalIds.append(out);
        return nbl::Result::SUCCESS;
    }));

    QVariantMap fields = params.find("fields")->toMap();
    const nbl::InstallationType* it = _mContext.get<nbl::InstallationType>(nbl::StringView(type.data(), type.length()));

    if(it) {
        NBL_CHECK_OR_ROLLBACK(databaseConnection(), _applyInstallationCommand(&_mContext, meta, it, fields, externalIds));
    }

    for(QVariant qs : params.find("addedInstallationTags")->toList()) {
        tagInstallation(meta.id, qs.toLongLong());
    }
    for(QVariant qs : params.find("removedInstallationTags")->toList()) {
        untagInstallation(meta.id, qs.toLongLong());
    }

    NBL_CHECK(databaseConnection()->commitTransaction());

    return nbl::Result::SUCCESS;
}

int ApplicationContext::deleteInstallation(qint64 installationId) {
    return nbl::removeInstallation(databaseConnection(), installationId);
}

