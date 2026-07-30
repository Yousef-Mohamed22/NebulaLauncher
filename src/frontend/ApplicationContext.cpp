#include "ApplicationContext.hpp"

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Time.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Assert.hpp>
#include <Platform/File.hpp>
#include <Platform/Process.hpp>
#include <Platform/StringBuilder.hpp>
#include <Data/Model.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/PluginContext.hpp>
#include <Codec/Json.hpp>
#include <Core/Span.hpp>
#include <Data/Database.hpp>

#include <QString>
#include <QDesktopServices>
#include <QColor>
#include <qcontainerfwd.h>
#include <qjsonparseerror.h>
#include <qprocess.h>

#include <openssl/evp.h>
#include <qvariant.h>

#include "Models/NblInstallationDetails.hpp"
#include "NblProcessHandle.hpp"


using namespace Qt::StringLiterals;

namespace {


nbl::Result pushImage(QString url, QString repo, QString* outPath) {

    if(url.isEmpty()) {
        *outPath = url;
        return nbl::Result::SUCCESS;
    }

    // Determine file url
    if(!url.startsWith("file://")) {
        QByteArray arr = url.toUtf8();
        nbl::log(nbl::LogLevel::ERROR, "Invalid URL: ", nbl::StringView(arr.data()), arr.length());
        return nbl::Result::INVALID_ARGUMENT;
    }
    
    url = url.mid(7);
    QFile file(url);

    QByteArray fileNameArr = url.toUtf8();
    nbl::StringView fileNameView = nbl::StringView(fileNameArr.data(), fileNameArr.length());

    if(!file.exists()) {
        nbl::log(nbl::LogLevel::ERROR, "File not found: ", fileNameView);
        return nbl::Result::INVALID_ARGUMENT;
    }
    if(!file.open(QIODevice::ReadOnly)) { 
        nbl::log(nbl::LogLevel::ERROR, "Could not open file: ", fileNameView);
        return nbl::Result::IO_ERROR;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        nbl::log(nbl::LogLevel::ERROR, "Unable to initialize EVP_MD_CTX");
        return nbl::Result::IO_ERROR;
    }

    if(EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        nbl::log(nbl::LogLevel::ERROR, "Unable to initialize MD5 digest!");
        return nbl::Result::IO_ERROR;
    }

    u8 hash[16] = {};

    char dataBuffer[4096] = {};
    while(!file.atEnd()) {
        qint64 bytesRead;
        if((bytesRead = file.read(dataBuffer, 4096)) == -1) {
            EVP_MD_CTX_free(ctx);
            nbl::log(nbl::LogLevel::ERROR, "Unable to read from file!");
            return nbl::Result::IO_ERROR;
        }
        if(bytesRead == 0) {
            break;
        }

        if(EVP_DigestUpdate(ctx, dataBuffer, bytesRead) != 1) {
            EVP_MD_CTX_free(ctx);
            nbl::log(nbl::LogLevel::ERROR, "Unable to update MD5 digest!");
            return nbl::Result::IO_ERROR;
        }
    }

    u32 outputLen = 16;
    if(EVP_DigestFinal_ex(ctx, hash, &outputLen) != 1) {
        EVP_MD_CTX_free(ctx);
        nbl::log(nbl::LogLevel::ERROR, "Unable to compute MD5 digest!");
        return nbl::Result::IO_ERROR;
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
        return nbl::Result::IO_ERROR;
    }

    return nbl::Result::SUCCESS;
}

}


void ApplicationContext::_initialize() {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    QByteArray qDataDir = dataFolder().toUtf8();
    nbl::StringView nblDataDir = nbl::StringView(qDataDir.data(), qDataDir.length());

    NBL_ASSERT(nbl::makeDirectories(nblDataDir).success());

    nbl::StringView path;
    NBL_ASSERT(nbl::StringBuilder(&frame).append(nblDataDir).append("/nebula.db").build(&path).success());

    NBL_ASSERT(nbl::PluginContext::create(path, {
        .pfnNotify = [](void* userData, nbl::NotificationType type, nbl::StringView title, nbl::StringView message, nbl::Optional<nbl::StringView> icon, nbl::Optional<u32> duration) {
        
            ApplicationContext* ctx = static_cast<ApplicationContext*>(userData);

            QVariantMap map = {};
            map["type"] = type == nbl::NotificationType::ERROR ? "error" : "info";
            map["title"] = fromNbl(title);
            map["message"] = fromNbl(message);
            if(icon.hasValue()) map["icon"] = fromNbl(*icon);
            if(duration.hasValue()) map["duration"] = *duration;
            
            ctx->pushNotification(map);

        },
        .userData = this
    }, &_mContext).success());

    _mContext.registerInstallationType(nbl::InstallationType::COMMAND);

    nbl::StringView applicationPath = nbl::getBinaryPath();
    usize lastSlash = 0;
    NBL_ASSERT(applicationPath.findLast('/', &lastSlash).success());

    nbl::StringView pluginFolder;
    NBL_ASSERT(nbl::StringBuilder(&frame).append(applicationPath.slice(0, lastSlash)).append("/plugins").buildNullTerminated(&pluginFolder).success());
    NBL_ASSERT(nbl::makeDirectories(pluginFolder).success());

    NBL_ASSERT(nbl::PluginLoader::create(&_mContext, pluginFolder, &_mLoader).success());
    NBL_ASSERT(_mLoader.loadAll().success());

    NBL_ASSERT(nbl::DatabaseConnection::open(path, &_mConnection).success());
    NBL_ASSERT(nbl::initDatabase(&_mConnection).success());
}


QString ApplicationContext::dataFolder() const {
    static QString _dataFolder = {};
    if(_dataFolder.isEmpty()) {
        if(qEnvironmentVariableIsSet("NBL_DATA_FOLDER")) {
            _dataFolder = qEnvironmentVariable("NBL_DATA_FOLDER");
        } else {
            nbl::StringView dataDir = nbl::getDataDirectory();
            _dataFolder = QString::fromUtf8(dataDir.data, dataDir.length).append("/NebulaLauncher");
        }
    }
    return _dataFolder;
}


NblProcessHandle* ApplicationContext::launchInstallation(i64 installationId) {

    if(!_mConnection.isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot launch installation, database connection is not valid");
        return nullptr;
    }

    if(_mRunningProcesses.contains(installationId)) {
        NblProcessHandle* handle = _mRunningProcesses[installationId];
        if(handle->isRunning()) {
            _mContext.pushNotification(nbl::NotificationType::ERROR, "Cannot start installation", "A process for this installation is already running!");
            return nullptr;
        }

        delete handle;
    }

    NblProcessHandle* handle = new NblProcessHandle();
    QQmlEngine::setObjectOwnership(handle, QQmlEngine::CppOwnership);

    connect(handle, &NblProcessHandle::stopped, this, [this, installationId, handle]() {
        _mRunningProcesses.remove(installationId);
        delete handle;
    });

    _mRunningProcesses[installationId] = handle;

    nbl::Result res = nbl::getInstallationMeta(&_mConnection, installationId, [&](const nbl::InstallationMeta& meta) -> nbl::Result {

        const nbl::InstallationType* installType = _mContext.getInstallationType(meta.type);
        if(installType == nullptr) {
            nbl::log(nbl::LogLevel::ERROR, "Unknown installation type: ", meta.type);
            return nbl::Result::INVALID_STATE;
        }

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::Span<nbl::FieldValue> values = {
            .data = nullptr,
            .length = installType->fields.length
        };

        if(installType->fields.length > 0) {
            frame.allocate(&values.data, installType->fields.length);

            nbl::JsonElement value = {};
            NBL_CHECK(nbl::parseJson(nbl::StringReader(meta.command), &frame, &value));
            NBL_CHECK(nbl::decodeFieldValues(installType, value.object, values));
        }
        NBL_CHECK(installType->launch(&_mContext, values, installationId, handle->handle()));
        NBL_CHECK(handle->initLogging());
        handle->check();

        return nbl::Result::SUCCESS;
    });
    if(res.error()) {
        delete handle;
        return nullptr;
    }

    return handle;
}

NblProcessHandle* ApplicationContext::getRunningInstallation(i64 installationId) {
    if(!_mRunningProcesses.contains(installationId)) return nullptr;
    return _mRunningProcesses[installationId];
}

int ApplicationContext::openInstallationFolder(i64 installationId) {
 
    if(!_mConnection.isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot open installation folder, database connection is not valid");
        return -1;
    }

    NblProcessHandle* handle = new NblProcessHandle();

    nbl::Result res = nbl::getInstallationMeta(&_mConnection, installationId, [&](const nbl::InstallationMeta& meta) -> nbl::Result {

        const nbl::InstallationType* installType = _mContext.getInstallationType(meta.type);
        if(installType == nullptr) {
            nbl::log(nbl::LogLevel::ERROR, "Unknown installation type: ", meta.type);
            return nbl::Result::INVALID_STATE;
        }

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::Span<nbl::FieldValue> values = {
            .data = nullptr,
            .length = installType->fields.length
        };

        frame.allocate(&values.data, installType->fields.length);

        nbl::JsonElement value = {};
        NBL_CHECK(nbl::parseJson(nbl::StringReader(meta.command), &frame, &value));
        NBL_CHECK(nbl::decodeFieldValues(installType, value.object, values));

        nbl::StringView folder = {};
        NBL_CHECK(installType->getFolder(&_mContext, values, installationId, &folder));

        if(folder.empty()) {
            _mContext.pushNotification(nbl::NotificationType::ERROR, "Could not open folder", "The installation did not specify a valid installation path");
            return nbl::Result::SUCCESS;
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(fromNbl(folder)));
        return nbl::Result::SUCCESS;
    });

    if(res.error()) {
        _mContext.pushNotification(nbl::NotificationType::ERROR, "Could not open folder", "An error occurred while getting the installation folder.");
    }

    return res;
}

NblInstallationDetails* ApplicationContext::getInstallationDetails(i64 installationId) {

    if(!_mConnection.isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot get installation details, database connection is not valid");
        return nullptr;
    }

    NblInstallationDetails* out;

    if(nbl::getInstallationDetails(&_mConnection, installationId, [&](const nbl::InstallationDetails& details) {

         out = new NblInstallationDetails(
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
    }).error()) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to get installation details for installation with ID ", installationId);
        return nullptr;
    }

    return out;

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
            .banner = nbl::StringView(banner.constData(), banner.length()),
            .poster = nbl::StringView(poster.constData(), poster.length()),
            .icon = nbl::StringView(icon.constData(), icon.length()),
            .logo = nbl::StringView(logo.constData(), logo.length()),
            .logoPosition = position
        };

        NBL_CHECK(nbl::addDisplay(&_mConnection, meta, &dId));

    } else {
        // Get display id from game
        NBL_CHECK(nbl::getGameDetails(&_mConnection, gId, [&](const nbl::GameDetails& meta) {
            dId = meta.display.id;
            return nbl::Result::SUCCESS;
        }));
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
            .title = nbl::StringView(title.constData(), title.length()),
            .description = nbl::StringView(description.constData(), description.length()),
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
        .type = nbl::StringView(type.data(), type.length()),
        .command = nbl::StringView(command.data(), command.length()),
    };

    QByteArray installationTitle = params.find("title")->toString().toUtf8();
    QByteArray installationDesc = params.find("description")->toString().toUtf8();

    meta.title = nbl::StringView(installationTitle.data(), installationTitle.length());
    meta.description = nbl::StringView(installationDesc.data(), installationDesc.length());

    NBL_CHECK(nbl::addInstallation(&_mConnection, meta, &meta.id));

    for(QVariant tag : params.find("addedInstallationTags")->toList()) {
        tagInstallation(meta.id, tag.toLongLong());
    }
    for(QVariant tag : params.find("removedInstallationTags")->toList()) {
        untagInstallation(meta.id, tag.toLongLong());
    }
    for(QVariant tag : params.find("addedGameTags")->toList()) {
        tagGame(realGId, tag.toLongLong());
    }
    for(QVariant tag : params.find("removedGameTags")->toList()) {
        untagGame(realGId, tag.toLongLong());
    }

    NBL_CHECK(_mConnection.commitTransaction());
    return 0;
}

int ApplicationContext::updateInstallation(int64_t installationId, const QVariantMap& params) {

    bool okay = false;
    i64 dId = params.find("displayId")->toLongLong(&okay);
    if(okay) {
        if(params.find("displayOverride")->toBool()) {

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
                .banner = nbl::StringView(banner.constData(), banner.length()),
                .poster = nbl::StringView(poster.constData(), poster.length()),
                .icon = nbl::StringView(icon.constData(), icon.length()),
                .logo = nbl::StringView(logo.constData(), logo.length()),
                .logoPosition = position
            };

            if(dId == 0) { // Newly overridden
                NBL_CHECK(nbl::addDisplay(&_mConnection, meta, &dId));
            } else {
                NBL_CHECK(nbl::updateDisplay(&_mConnection, dId, meta));
            }

        } else if(dId != 0) {

            NBL_CHECK(nbl::removeDisplay(&_mConnection, dId));

        }
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
        .title = nbl::StringView(installationTitle.data(), installationTitle.length()),
        .description = nbl::StringView(installationDesc.data(), installationDesc.length()),
        .type = nbl::StringView(type.data(), type.length()),
        .command = nbl::StringView(cmd.data(), cmd.length())
    };

    nbl::updateInstallation(&_mConnection, meta);

    for(QVariant qs : params.find("addedInstallationTags")->toList()) {
        tagInstallation(meta.id, qs.toLongLong());
    }
    for(QVariant qs : params.find("removedInstallationTags")->toList()) {
        untagInstallation(meta.id, qs.toLongLong());
    }

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
        nbl::log(nbl::LogLevel::ERROR, "Unable to save settings to ", nbl::StringView(arr.data()), arr.length());
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

// Tabs

namespace {

nbl::Result _pushFilters(nbl::DatabaseConnection* connection, i64 tabId, const QVariantList& filters) {

    for(const QVariant& filterVar : filters) {

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());

        QVariantMap filter = filterVar.toMap();
        i64 filterId = filter["id"].toLongLong();
        
        QByteArray name = filter["name"].toByteArray();
        QString icon = filter["icon"].toString();
        bool colorizeIcon = filter["colorizeIcon"].toBool();
        bool multiLine = filter["multiLine"].toBool();
        bool useIcons = filter["useIcons"].toBool();
        bool showTitles = filter["showTitles"].toBool();
        i64 priority = filter["priority"].toLongLong();

        if(icon.endsWith(".png") || icon.endsWith(".svg")) {
            NBL_CHECK(pushImage(icon, "inline_icon", &icon));
        }
        QByteArray realIcon = icon.toUtf8();

        nbl::InstallationSortBy sortBy = static_cast<nbl::InstallationSortBy>(filter["sortBy"].toInt());
        nbl::SortOrder sortOrder = static_cast<nbl::SortOrder>(filter["sortOrder"].toInt());
        
        u64 minReleaseDate = filter["minReleaseDate"].toULongLong();
        u64 maxReleaseDate = filter["minReleaseDate"].toULongLong();
        f64 minRating = filter["minRating"].toDouble();
        f64 maxRating = filter["minRating"].toDouble();
        u64 minPlayTime = filter["minTime"].toULongLong();
        u64 maxPlayTime = filter["minTime"].toULongLong();


        QVariantList setsList = filter["tags"].toList();
        nbl::Span<nbl::ConstSpan<i64>> tagSets{};
        if(setsList.length() > 0) {
            NBL_CHECK(frame.allocate(&tagSets.data, setsList.length()));
            for(QVariant var : setsList) {

                QVariantList list = var.toList();

                nbl::Span<i64> tags{};
                NBL_CHECK(frame.allocate(&tags.data, list.length()));
                for(QVariant tId : list) {
                    tags[tags.length++] = tId.toLongLong();
                }
                tagSets[tagSets.length++] = tags;
            }
        }


        QVariantList excludeList = filter["excludeTags"].toList();
        nbl::Span<i64> exclude{};
        if(excludeList.length() > 0) {
            NBL_CHECK(frame.allocate(&exclude.data, excludeList.length()));
            for(QVariant var : excludeList) {
                exclude[exclude.length++] = var.toLongLong();
            }
        }

        nbl::FilterMeta meta = {
            .filterId = filterId,
            .tabId = tabId,
            .priority = priority,
            .name = nbl::StringView(name.data(), name.length()),
            .icon = nbl::StringView(realIcon.data(), realIcon.length()),
            .colorizeIcon = colorizeIcon,
            .multiLine = multiLine,
            .showTitles = showTitles,
            .useIcons = useIcons,
            .criteria = {
                .tags = tagSets,
                .excludedTags = exclude,
                .minReleaseDate = minReleaseDate,
                .maxReleaseDate = maxReleaseDate,
                .minRating = minRating,
                .maxRating = maxRating,
                .minPlayTime = minPlayTime,
                .maxPlayTime = maxPlayTime,
                .sortBy = sortBy,
                .sortOrder = sortOrder
            }
        };

        if(meta.filterId == 0) {
            NBL_CHECK(nbl::addFilter(connection, meta, &meta.filterId));
        } else {
            NBL_CHECK(nbl::updateFilter(connection, meta));
        }
    }

    return nbl::Result::SUCCESS;
}

}

int ApplicationContext::addTab(const QVariantMap& params) {

    QByteArray name = params["name"].toByteArray();
    QString icon = params["icon"].toString();
    bool colorizeIcon = params["colorizeIcon"].toBool();
    bool hidden = params["hidden"].toBool();

    nbl::DatabaseConnection* c = &_mConnection;
    if(icon.endsWith(".png") || icon.endsWith(".svg")) {
        NBL_CHECK_OR_ROLLBACK(c, pushImage(icon, "inline_icon", &icon));
    }

    QByteArray realIcon = icon.toUtf8();

    nbl::TabMeta meta = {
        .name = nbl::StringView(name.data(), name.length()),
        .icon = nbl::StringView(realIcon.data(), realIcon.length()),
        .colorizeIcon = colorizeIcon,
        .hidden = hidden
    };

    NBL_CHECK(_mConnection.beginTransaction());

    NBL_CHECK_OR_ROLLBACK(c, nbl::addTab(&_mConnection, meta, &meta.tabId));
    NBL_CHECK_OR_ROLLBACK(c, _pushFilters(&_mConnection, meta.tabId, params["filters"].toList()));

    NBL_CHECK(c->commitTransaction());

    return nbl::Result::SUCCESS;
}
int ApplicationContext::updateTab(qint64 id, const QVariantMap& params) {

    QByteArray name = params["name"].toByteArray();
    QString icon = params["icon"].toString();
    bool colorizeIcon = params["colorizeIcon"].toBool();
    bool hidden = params["hidden"].toBool();

    nbl::DatabaseConnection* c = &_mConnection;
    if(icon.endsWith(".png") || icon.endsWith(".svg")) {
        NBL_CHECK_OR_ROLLBACK(c, pushImage(icon, "inline_icon", &icon));
    }

    QByteArray realIcon = icon.toUtf8();

    nbl::TabMeta meta = {
        .tabId = static_cast<i64>(id),
        .name = nbl::StringView(name.data(), name.length()),
        .icon = nbl::StringView(realIcon.data(), realIcon.length()),
        .colorizeIcon = colorizeIcon,
        .hidden = hidden
    };

    NBL_CHECK_OR_ROLLBACK(c, _mConnection.beginTransaction());
    NBL_CHECK_OR_ROLLBACK(c, nbl::updateTab(&_mConnection, meta));

    NBL_CHECK_OR_ROLLBACK(c, _pushFilters(&_mConnection, meta.tabId, params["filters"].toList()));

    for(const QVariant& removed : params["removedFilters"].toList()) {
        NBL_CHECK_OR_ROLLBACK(c, nbl::removeFilter(c, removed.toLongLong()));
    }

    NBL_CHECK(c->commitTransaction());
    return nbl::Result::SUCCESS;
}
int ApplicationContext::deleteTab(qint64 id) { 

    return nbl::removeTab(&_mConnection, id);
}

QVariantMap ApplicationContext::getTabMeta(qint64 id) {

    QVariantMap out = {};
    out["id"] = id;

    if(nbl::getTabMeta(&_mConnection, static_cast<i64>(id), [&](const nbl::TabMeta& meta) -> nbl::Result {

        out["name"] = fromNbl(meta.name);
        out["icon"] = fromNbl(meta.icon);
        out["colorizeIcon"] = meta.colorizeIcon;
        out["hidden"] = meta.hidden;

        return nbl::Result::SUCCESS;
    }).error()) {
        out["id"] = 0;
        return out;
    }

    QVariantList filters = {};

    if(nbl::getFiltersForTab(&_mConnection, id, 0, INT32_MAX, [&](const nbl::FilterMeta& meta) -> nbl::Result {

        QVariantMap filter = {};

        filter["id"] = static_cast<qint64>(meta.filterId);
        filter["name"] = fromNbl(meta.name);
        filter["icon"] = fromNbl(meta.icon);
        filter["colorizeIcon"] = meta.colorizeIcon;
        filter["multiLine"] = meta.multiLine;
        filter["useIcons"] = meta.useIcons;
        filter["showTitles"] = meta.showTitles;
        

        nbl::getFilterCriteria(&_mConnection, meta.filterId, [&](const nbl::InstallationSearchCriteria& criteria) -> nbl::Result {

            filter["sortBy"] = static_cast<i32>(criteria.sortBy);
            filter["sortOrder"] = static_cast<i32>(criteria.sortOrder);
            filter["minReleaseDate"] = static_cast<quint64>(criteria.minReleaseDate);
            filter["maxReleaseDate"] = static_cast<quint64>(criteria.maxReleaseDate);
            filter["minRating"] = criteria.minRating;
            filter["maxRating"] = criteria.maxRating;
            filter["minTime"] = static_cast<quint64>(criteria.minPlayTime);
            filter["maxTime"] = static_cast<quint64>(criteria.maxPlayTime);

            QVariantList tags = {};
            for(nbl::ConstSpan<i64> tagSet : criteria.tags) {
                QVariantList tagsInSet = {};
                for(i64 tag : tagSet) {
                    tagsInSet.append(static_cast<qint64>(tag));
                }
                tags.append(QVariant(tagsInSet));
            }

            QVariantList excludeTags = {};
            for(i64 excluded : criteria.excludedTags) {
                excludeTags.append(static_cast<qint64>(excluded));
            }

            filter["tags"] = tags;
            filter["excludeTags"] = excludeTags;

            return nbl::Result::SUCCESS;
        });

        filters.append(filter);

        return nbl::Result::SUCCESS;
    }).error()) {
        out["id"] = 0;
        return out;
    }

    out["filters"] = filters;

    return out;
}


// Tags
qint64 ApplicationContext::addTag(QString name, QColor color) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.data(), arr.length());
    i64 id = 0;
    if(nbl::addTag(&_mConnection, nblName, color.rgba(), &id).error()) {
        return 0;
    }
    return static_cast<qint64>(id);
}

qint64 ApplicationContext::findTagId(QString name) {

    qint64 out;
    
    QByteArray utf = name.toUtf8();
    nbl::Result rc = nbl::findTag(&_mConnection, nbl::StringView(utf.data(), utf.length()), [&](const nbl::TagMeta& meta) {
        out = meta.id;
        return nbl::Result::SUCCESS;
    });

    if(rc == nbl::Result::NOT_FOUND) return 0;
    return out;
}

int ApplicationContext::removeTag(qint64 tag) {
    return nbl::removeTag(&_mConnection, tag);
}

int ApplicationContext::renameTag(qint64 tag, QStringView newName) {
    QByteArray newArr = newName.toUtf8();
    nbl::StringView nblNewName = nbl::StringView(newArr.data(), newArr.length());
    return nbl::renameTag(&_mConnection, tag, nblNewName);
}

int ApplicationContext::recolorTag(qint64 tag, QColor color) {
    return nbl::setTagColor(&_mConnection, tag, color.rgba());
}

int ApplicationContext::addTagParent(qint64 tag, qint64 parent) {
    return nbl::addParentTag(&_mConnection, parent, tag);
}

int ApplicationContext::tagGame(qint64 gameId, qint64 tag) {
    return nbl::addGameTag(&_mConnection, gameId, tag);
}

int ApplicationContext::untagGame(qint64 gameId, qint64 tag) {
    return nbl::removeGameTag(&_mConnection, gameId, tag);
}

int ApplicationContext::tagInstallation(qint64 installationId, qint64 tag) {
    return nbl::addInstallationTag(&_mConnection, installationId, tag);
}

int ApplicationContext::untagInstallation(qint64 installationId, qint64 tag) {
    return nbl::removeInstallationTag(&_mConnection, installationId, tag);
}

int ApplicationContext::updateTags(QVariantList modifiedTags) {
    for(QVariant tag : modifiedTags) {

        QVariantMap map = tag.toMap();

        qint64 id = map["id"].toLongLong();

        QString originalName = map["originalName"].toString();
        QString name = map["name"].toString();
        QColor color = map["color"].value<QColor>();
        QVariantList newParents = map["addedParents"].toList();
        QVariantList removedParents = map["removedParents"].toList();

        NBL_CHECK(_mConnection.beginTransaction());

        if(originalName != name) {
            renameTag(id, name);
        }

        recolorTag(id, color);

        for(QVariant parent : newParents) {
            nbl::addParentTag(&_mConnection, parent.toLongLong(), id);
        }
        for(QVariant parent : removedParents) {
            nbl::removeParentTag(&_mConnection, parent.toLongLong(), id);
        }

        NBL_CHECK(_mConnection.commitTransaction());
    }

    return 0;
}


QVariantList ApplicationContext::getTagsFromIds(QVariantList ids) {
    QList<QVariant> out = {};

    QByteArray namesJson = QJsonDocument(QJsonArray::fromVariantList(ids)).toJson(QJsonDocument::JsonFormat::Compact);

    nbl::getTagsFromIds(&_mConnection, nbl::StringView(namesJson.data(), namesJson.length()), [&](const nbl::TagMeta& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });
    return out;
}

QList<QVariant> ApplicationContext::getGameTags(qint64 gameId) {
    QList<QVariant> out = {};
    nbl::getGameTags(&_mConnection, gameId, [&](const nbl::TagMeta& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });
    return out;
}

QList<QVariant> ApplicationContext::getInstallationTags(qint64 installationId) {
    QList<QVariant> out = {};
    nbl::getDirectInstallationTags(&_mConnection, installationId, [&](const nbl::TagMeta& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });

    return out;
}

QList<QVariant> ApplicationContext::getTagParents(qint64 tag) {
    QList<QVariant> out = {};

    nbl::parents(&_mConnection, tag, [&](const nbl::TagMeta& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });

    return out;
}


int ApplicationContext::addGame(QVariantMap params) {

    i64 dId = 0;

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

    nbl::DisplayMeta displayMeta = {
        .banner = nbl::StringView(banner.constData(), banner.length()),
        .poster = nbl::StringView(poster.constData(), poster.length()),
        .icon = nbl::StringView(icon.constData(), icon.length()),
        .logo = nbl::StringView(logo.constData(), logo.length()),
        .logoPosition = position
    };

    NBL_CHECK(nbl::addDisplay(&_mConnection, displayMeta, &dId));

    QByteArray title = params.find("title")->toString().toUtf8();
    QByteArray description = params.find("description")->toString().toUtf8();
    u64 steamId = params.find("steamId")->toUInt();
    u64 igdbId = params.find("igdbId")->toUInt();
    u64 releaseDate = params.find("releaseDate")->toULongLong();
    f64 rating = params.find("rating")->toDouble();

    nbl::GameMeta gameMeta = {
        .id = 0,
        .displayId = dId,
        .title = nbl::StringView(title.constData(), title.length()),
        .description = nbl::StringView(description.constData(), description.length()),
        .steamId = steamId,
        .igdbId = igdbId,
        .releaseDate = releaseDate,
        .rating = rating
    };

    i64 gId = 0;
    NBL_CHECK(nbl::addGame(&_mConnection, gameMeta, &gId));

    for(QVariant qs : params.find("tags")->toList()) {
        tagGame(gId, qs.toLongLong());
    }

    return 0;
}
int ApplicationContext::updateGame(QVariantMap params) {

    bool ok = true;
    i64 gId = params.find("id")->toLongLong(&ok);
    i64 dId = params.find("displayId")->toLongLong(&ok);

    if(!ok) {
        return nbl::Result::INVALID_ARGUMENT;
    }

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

    nbl::DisplayMeta displayMeta = {
        .banner = nbl::StringView(banner.constData(), banner.length()),
        .poster = nbl::StringView(poster.constData(), poster.length()),
        .icon = nbl::StringView(icon.constData(), icon.length()),
        .logo = nbl::StringView(logo.constData(), logo.length()),
        .logoPosition = position
    };

    NBL_CHECK(nbl::updateDisplay(&_mConnection, dId, displayMeta));

    QByteArray title = params.find("title")->toString().toUtf8();
    QByteArray description = params.find("description")->toString().toUtf8();
    u64 steamId = params.find("steamId")->toUInt();
    u64 igdbId = params.find("igdbId")->toUInt();
    u64 releaseDate = params.find("releaseDate")->toULongLong();
    f64 rating = params.find("rating")->toDouble();

    nbl::GameMeta gameMeta = {
        .id = 0,
        .displayId = dId,
        .title = nbl::StringView(title.constData(), title.length()),
        .description = nbl::StringView(description.constData(), description.length()),
        .steamId = steamId,
        .igdbId = igdbId,
        .releaseDate = releaseDate,
        .rating = rating
    };

    NBL_CHECK(nbl::updateGame(&_mConnection, gId, gameMeta));

    for(QVariant qs : params.find("tags")->toList()) {
        tagGame(gId, qs.toLongLong());
    }
    for(QVariant qs : params.find("removedTags")->toList()) {
        untagGame(gId, qs.toLongLong());
    }
    return 0;
}

QVariantMap ApplicationContext::getGameDetails(qint64 gameId) {

    QVariantMap out = {};

    nbl::Result res = nbl::getGameDetails(&_mConnection, gameId, [&](const nbl::GameDetails& details) {

        out["id"] = static_cast<qint64>(details.game.id);
        out["displayId"] = static_cast<qint64>(details.display.id);
        out["title"] = fromNbl(details.game.title);
        out["description"] = fromNbl(details.game.description);
        out["poster"] = fromNbl(details.display.poster);
        out["icon"] = fromNbl(details.display.icon);
        out["logo"] = fromNbl(details.display.logo);
        out["banner"] = fromNbl(details.display.banner);
        out["logoPosition"] = static_cast<int>(details.display.logoPosition);
        out["releaseDate"] = static_cast<quint64>(details.game.releaseDate);
        out["rating"] = details.game.rating;
        out["steamId"] = static_cast<quint64>(details.game.steamId);
        out["igdbId"] = static_cast<quint64>(details.game.igdbId);

        return nbl::Result::SUCCESS;
    });

    if(res != 0) {
        nbl::log(nbl::LogLevel::ERROR, "An error occurred while looking up game details! ", res);
        return {};
    }

    out["tags"] = getGameTags(gameId);
    return out;
}

int ApplicationContext::deleteGame(qint64 gameId) {
    return nbl::removeGame(&_mConnection, gameId);
}


