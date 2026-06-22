#include "ApplicationContext.hpp"

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Time.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Assert.hpp>
#include <Platform/File.hpp>
#include <Platform/Process.hpp>
#include <Data/Model.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/PluginContext.hpp>

#include <QString>
#include <qcontainerfwd.h>
#include <qjsonparseerror.h>
#include <qprocess.h>

#include <openssl/evp.h>

#include "Models/NblInstallationDetails.hpp"

namespace {


nbl::Result pushImage(QString url, QString repo, QString* outPath) {

    if(url.isEmpty()) {
        *outPath = url;
        return nbl::Result::SUCCESS;
    }

    // Determine file url
    if(!url.startsWith("file://")) {
        QByteArray arr = url.toUtf8();
        nbl::log(nbl::LogLevel::ERROR, "Invalid URL: ", nbl::StringView(arr.length(), arr.data()));
        return nbl::error::ILLEGAL_ARGUMENT;
    }
    
    url = url.mid(7);
    QFile file(url);

    QByteArray fileNameArr = url.toUtf8();
    nbl::StringView fileNameView = nbl::StringView(fileNameArr.length(), fileNameArr.data());

    if(!file.exists()) {
        nbl::log(nbl::LogLevel::ERROR, "File not found: ", fileNameView);
        return nbl::error::ILLEGAL_ARGUMENT;
    }
    if(!file.open(QIODevice::ReadOnly)) { 
        nbl::log(nbl::LogLevel::ERROR, "Could not open file: ", fileNameView);
        return nbl::error::IO_ERROR;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        nbl::log(nbl::LogLevel::ERROR, "Unable to initialize EVP_MD_CTX");
        return nbl::error::IO_ERROR;
    }

    if(EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        nbl::log(nbl::LogLevel::ERROR, "Unable to initialize MD5 digest!");
        return nbl::error::IO_ERROR;
    }

    u8 hash[16] = {};

    nbl::log(nbl::LogLevel::INFO, "Calculating MD5 for ", fileNameView);

    char dataBuffer[4096] = {};
    while(!file.atEnd()) {
        qint64 bytesRead;
        if((bytesRead = file.read(dataBuffer, 4096)) == -1) {
            EVP_MD_CTX_free(ctx);
            nbl::log(nbl::LogLevel::ERROR, "Unable to read from file!");
            return nbl::error::IO_ERROR;
        }
        if(bytesRead == 0) {
            break;
        }

        if(EVP_DigestUpdate(ctx, dataBuffer, bytesRead) != 1) {
            EVP_MD_CTX_free(ctx);
            nbl::log(nbl::LogLevel::ERROR, "Unable to update MD5 digest!");
            return nbl::error::IO_ERROR;
        }
    }

    u32 outputLen = 16;
    if(EVP_DigestFinal_ex(ctx, hash, &outputLen) != 1) {
        EVP_MD_CTX_free(ctx);
        nbl::log(nbl::LogLevel::ERROR, "Unable to compute MD5 digest!");
        return nbl::error::IO_ERROR;
    }

    QString extension = url.sliced(url.lastIndexOf(".") - 1);
    QString fileName = {};
    fileName.reserve(32 + 1 + extension.length());

    for(usize i = 0 ; i < 16 ; i++) {
        if(i == 1) fileName += "/";
        fileName += QString::asprintf("%02x", hash[i]);
    }
    fileName += extension;

    EVP_MD_CTX_free(ctx);

    QString dataFolder = ApplicationContext::instance()->dataFolder();
    QString outputFile = dataFolder + "/media/" + repo + "/" + fileName;

    *outPath = fileName;

    QFile output(outputFile);
    if(output.exists()) return nbl::Result::SUCCESS;


    QDir dir(QFileInfo(outputFile).absolutePath());
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }

    if(!file.copy(output.filesystemFileName())) {
        nbl::log(nbl::LogLevel::ERROR, "Unable to copy file!");
        return nbl::error::IO_ERROR;
    }

    return nbl::Result::SUCCESS;
}


}


void ApplicationContext::_initialize() {

    NBL_ASSERT(nbl::Arena::create(1024 * 1024 * 10, &_mProcessArena).success());
    NBL_ASSERT(nbl::PoolView<nbl::ProcessHandle>::create(
        &_mProcessArena,
        &_mRunningInstallations
    ).success());

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringView dataDir;
    NBL_ASSERT(nbl::StringBuilder(&frame).append(nbl::getDataDirectory()).append("/NebulaLauncher").build(&dataDir).success());
    NBL_ASSERT(nbl::makeDirectories(dataDir).success());

    nbl::StringView path;
    NBL_ASSERT(nbl::StringBuilder(&frame).append(dataDir).append("/nebula.db").build(&path).success());

    NBL_ASSERT(nbl::PluginContext::create(path, &_mContext).success());
    _mContext.registerInstallationType(nbl::InstallationType::COMMAND);

    nbl::StringView applicationPath = nbl::getBinaryPath();
    usize lastSlash = 0;
    NBL_ASSERT(applicationPath.findLast('/', &lastSlash).success());

    nbl::StringView pluginFolder;
    NBL_ASSERT(nbl::StringBuilder(&frame).append(applicationPath.slice(0, lastSlash)).append("/plugins").buildNullTerminated(&pluginFolder).success());
    NBL_ASSERT(nbl::makeDirectories(pluginFolder).success());

    NBL_ASSERT(nbl::PluginLoader::create(&_mContext, pluginFolder, &_mLoader).success());
    NBL_ASSERT(_mLoader.loadAll().success());

}



nbl::Result ApplicationContext::_reload() {

    if(!_mConnection.isValid()) {

        NBL_CHECK(nbl::DatabaseConnection::open(_mContext.databasePath(), &_mConnection));
        NBL_CHECK(nbl::initDatabase(&_mConnection));
    }

    _mAllTabs.clear();

    // Get tabs from database
    NBL_CHECK(nbl::enumerateTabs(&_mConnection, [](void* userData, i64 id, nbl::StringView name, nbl::StringView icon, bool hidden) {

        NblTabRef ref(id, 
            QString::fromUtf8(name.data, name.length), 
            QString::fromUtf8(icon.data, icon.length)
        );

        static_cast<QList<NblTabRef>*>(userData)->append(ref);

        return nbl::Result::SUCCESS;
    }, &_mAllTabs));

    tabsUpdated();

    return nbl::Result::SUCCESS;

}

void ApplicationContext::launchInstallation(i64 installationId) {

    if(!_mConnection.isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot launch installation, database connection is not valid");
        return;
    }

    nbl::getInstallationMeta(&_mConnection, installationId, [](void* userData, const nbl::InstallationMeta& meta) -> nbl::Result {

        ApplicationContext* context = static_cast<ApplicationContext*>(userData);

        nbl::ProcessHandle* runningInstallation;
        NBL_CHECK(context->_mRunningInstallations.push(&runningInstallation));

        const nbl::InstallationType* installType = context->_mContext.getInstallationType(meta.type);
        if(installType == nullptr) {
            nbl::log(nbl::LogLevel::ERROR, "Unknown installation type: ", meta.type);
            return nbl::error::ILLEGAL_STATE;
        }

        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(meta.command.data, meta.command.length));
        if(!doc.isObject()) return nbl::error::ILLEGAL_STATE;

        QList<nbl::FieldValue> values;
        QList<QByteArray> strings;
        values.reserve(installType->fields.length);

        for(auto& field : installType->fields) {

            QByteArray qKey = QByteArray(field.key.data, field.key.length);
            QJsonValue val = doc[qKey];

            if(val.isNull()) {
                values.append(field.defaultValue);            
            } else {

                switch(field.type) {
                    case nbl::FieldType::STRING: {
                        QByteArray str = val.toString().toUtf8();
                        strings.append(str);
                        QByteArray& realStr = strings.last();
                        values.append(nbl::FieldValue { .stringValue = nbl::StringView(realStr.length(), realStr.data()) } );
                        break;
                    }
                    case nbl::FieldType::FLOAT: {
                        values.append(nbl::FieldValue { .floatValue = val.toDouble() } );
                        break;
                    }
                    case nbl::FieldType::INTEGER: {
                        values.append(nbl::FieldValue { .integerValue = val.toInteger() } );
                        break;
                    }
                    case nbl::FieldType::BOOLEAN: {
                        values.append(nbl::FieldValue { .booleanValue = val.toBool() } );
                        break;
                    }
                    case nbl::FieldType::ENUM: {
                        QByteArray name = val.toString().toUtf8();
                        nbl::StringView nameView = nbl::StringView(name.length(), name.data());
                        u32 index = field.enumData.index(nameView);
                        values.append(nbl::FieldValue { .enumValue = index });
                        break;
                    }
                }
            }
        }

        return installType->launch(&context->_mContext, { .data = values.data(), .length = static_cast<usize>(values.length()) }, meta.id, runningInstallation);
        
    }, this);
}

NblInstallationDetails* ApplicationContext::getInstallationDetails(i64 installationId) {

    if(!_mConnection.isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot get installation details, database connection is not valid");
        return nullptr;
    }

    NblInstallationDetails* details;

    if(nbl::getInstallationDetails(&_mConnection, installationId, [](void* userData, const nbl::InstallationDetails& details) {

         *static_cast<NblInstallationDetails**>(userData) = new NblInstallationDetails(
            details.installation.id,
            details.display.id,
            details.game.id,
            fromNbl(details.installation.type),
            fromNbl(details.installation.command),
            fromNbl(details.installation.title),
            fromNbl(details.installation.description),
            fromNbl(details.display.poster),
            fromNbl(details.display.banner),
            fromNbl(details.display.icon),
            fromNbl(details.display.logo),
            static_cast<u64>(details.display.logoPosition),
            details.installation.playTime,
            details.installation.playCount,
            details.installation.lastPlayed,
            details.installation.added,
            details.game.releaseDate,
            details.game.rating,
            details.game.steamId,
            details.game.igdbId,
            fromNbl(details.game.title),
            fromNbl(details.game.description)
        );

        return nbl::Result::SUCCESS;
    }, &details).error()) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to get installation details for installation with ID ", installationId);
        return nullptr;
    }

    return details;

}


int ApplicationContext::addInstallation(const QVariantMap& params) {

    bool ok;

    i64 gId = params.find("game")->toLongLong(&ok);
    if(!ok) return -1;

    NBL_CHECK(_mConnection.beginTransaction());

    bool addDisplay = gId == 0 || params.find("displayOverride")->toBool();

    i64 dId = 0;
    if(addDisplay) {
        // Add a new display
        QString posterUrl = params.find("poster")->toString();
        QString iconUrl = params.find("icon")->toString();
        QString logoUrl = params.find("logo")->toString();
        QString bannerUrl = params.find("banner")->toString();

        NBL_CHECK(pushImage(posterUrl, "poster", &posterUrl));
        NBL_CHECK(pushImage(iconUrl, "icon", &iconUrl));
        NBL_CHECK(pushImage(logoUrl, "logo", &logoUrl));
        NBL_CHECK(pushImage(bannerUrl, "banner", &bannerUrl));
        nbl::LogoPosition position = static_cast<nbl::LogoPosition>(params.find("logoPosition")->toUInt());

        QByteArray poster = posterUrl.toUtf8();
        QByteArray icon = iconUrl.toUtf8();
        QByteArray logo = logoUrl.toUtf8();
        QByteArray banner = bannerUrl.toUtf8();

        nbl::DisplayMeta meta = {
            .banner = nbl::StringView(banner.length(), banner.constData()),
            .poster = nbl::StringView(poster.length(), poster.constData()),
            .icon = nbl::StringView(icon.length(), icon.constData()),
            .logo = nbl::StringView(logo.length(), logo.constData()),
            .logoPosition = position
        };

        NBL_CHECK(nbl::addDisplay(&_mConnection, meta, &dId));

    } else {
        // Get display id from game
        NBL_CHECK(nbl::getGameDetails(&_mConnection, gId, [](void* userData, const nbl::GameDetails& meta) {
            *static_cast<i64*>(userData) = meta.display.id;
            return nbl::Result::SUCCESS;
        }, &dId));
    }

    i64 realGId = gId;
    if(gId == 0) {
        // Add a new game
        QByteArray title = params.find("gameTitle")->toString().toUtf8();
        QByteArray description = params.find("gameDescription")->toString().toUtf8();
        u64 steamId = params.find("steamId")->toUInt();
        u64 igdbId = params.find("igdbId")->toUInt();
        u64 releaseDate = params.find("releaseDate")->toUInt();
        f64 rating = params.find("rating")->toDouble();

        nbl::GameMeta meta = {
            .id = 0,
            .displayId = dId,
            .title = nbl::StringView(title.length(), title.constData()),
            .description = nbl::StringView(description.length(), description.constData()),
            .steamId = steamId,
            .igdbId = igdbId,
            .releaseDate = releaseDate,
            .rating = rating
        };

        NBL_CHECK(nbl::addGame(&_mConnection, meta, &realGId));
    }

    // Add a new installation
    QByteArray type = params.find("type")->toString().toUtf8();
    QByteArray command = params.find("command")->toString().toUtf8();

    nbl::InstallationMeta meta = {
        .id = 0,
        .displayId = gId == 0 ? 0 : dId,
        .gameId = realGId,
        .type = nbl::StringView(type.length(), type.data()),
        .command = nbl::StringView(command.length(), command.data()),
    };

    QByteArray installationTitle = params.find("title")->toString().toUtf8();
    QByteArray installationDesc = params.find("description")->toString().toUtf8();

    meta.title = nbl::StringView(installationTitle.length(), installationTitle.data());
    meta.description = nbl::StringView(installationDesc.length(), installationDesc.data());

    NBL_CHECK(nbl::addInstallation(&_mConnection, meta, nullptr));
    NBL_CHECK(_mConnection.commitTransaction());

    return 0;
}

int ApplicationContext::updateInstallation(int64_t installationId, const QVariantMap& params) {

    bool okay = false;
    i64 dId = params.find("displayId")->toLongLong(&okay);
    if(okay && params.find("displayOverride")->toBool()) {

        QByteArray title = params.find("gameTitle")->toString().toUtf8();
        QByteArray description = params.find("gameDescription")->toString().toUtf8();
        QString posterUrl = params.find("poster")->toString();
        QString bannerUrl = params.find("banner")->toString();
        QString logoUrl = params.find("logo")->toString();
        QString iconUrl = params.find("icon")->toString();

        NBL_CHECK(pushImage(posterUrl, "poster", &posterUrl));
        NBL_CHECK(pushImage(iconUrl, "icon", &iconUrl));
        NBL_CHECK(pushImage(logoUrl, "logo", &logoUrl));
        NBL_CHECK(pushImage(bannerUrl, "banner", &bannerUrl));
        nbl::LogoPosition position = static_cast<nbl::LogoPosition>(params.find("logoPosition")->toUInt());

        QByteArray poster = posterUrl.toUtf8();
        QByteArray icon = iconUrl.toUtf8();
        QByteArray logo = logoUrl.toUtf8();
        QByteArray banner = bannerUrl.toUtf8();

        nbl::DisplayMeta meta = {
            .banner = nbl::StringView(banner.length(), banner.constData()),
            .poster = nbl::StringView(poster.length(), poster.constData()),
            .icon = nbl::StringView(icon.length(), icon.constData()),
            .logo = nbl::StringView(logo.length(), logo.constData()),
            .logoPosition = position
        };

        NBL_CHECK(nbl::updateDisplay(&_mConnection, dId, meta));
    } else {
        dId = 0;
    }

    i64 gId = params.find("game")->toLongLong(&okay);
    if(!okay) return -1;

    QByteArray type = params.find("type")->toString().toUtf8();
    QByteArray cmd = params.find("command")->toString().toUtf8();
    QByteArray installationTitle = params.find("title")->toString().toUtf8();
    QByteArray installationDesc = params.find("description")->toString().toUtf8();

    nbl::InstallationMeta meta = {
        .id = installationId,
        .displayId = dId,
        .gameId = gId,
        .title = nbl::StringView(installationTitle.length(), installationTitle.data()),
        .description = nbl::StringView(installationDesc.length(), installationDesc.data()),
        .type = nbl::StringView(type.length(), type.data()),
        .command = nbl::StringView(cmd.length(), cmd.data())
    };

    nbl::updateInstallation(&_mConnection, meta);
    return nbl::Result::SUCCESS;
}

int ApplicationContext::deleteInstallation(int64_t installationId) {
    return nbl::removeInstallation(&_mConnection, installationId);
}

void ApplicationContext::saveSettings(const QVariantMap& settings) {

    QJsonObject obj = QJsonObject::fromVariantMap(settings);
    QJsonDocument doc(obj);

    QString filePath = dataFolder() + "/frontend.json";
    QFile file(filePath);

    if(file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        QByteArray arr = filePath.toUtf8();
        nbl::log(nbl::LogLevel::ERROR, "Unable to save settings to ", nbl::StringView(arr.length(), arr.data()));
    }
}

QVariantMap ApplicationContext::loadSettings() {

    QFile file(dataFolder() + "/frontend.json");
    if(!file.exists()) {
        return QVariantMap();
    }

    if(!file.open(QIODevice::ReadOnly)) {
        nbl::log(nbl::LogLevel::ERROR, "Unable to load settings file!");
        return QVariantMap();
    }
    
    QJsonParseError err = {};
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    
    if(err.error != QJsonParseError::NoError || !doc.isObject()) {
        nbl::log(nbl::LogLevel::ERROR, "An error occurred while loading the settings file!");
        return QVariantMap();
    }

    return doc.object().toVariantMap();
}
