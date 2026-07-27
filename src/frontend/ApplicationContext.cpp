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
#include <QDesktopServices>
#include <QColor>
#include <qcontainerfwd.h>
#include <qjsonparseerror.h>
#include <qprocess.h>

#include <openssl/evp.h>

#include "Codec/Json.hpp"
#include "Data/Database.hpp"
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


void _pushNotification(ApplicationContext* context, QString type, QString title, QString message) {
    QVariantMap map = {};
    map["type"] = type;
    map["title"] = title;
    map["message"] = message;
    context->pushNotification(map);
}


}


void ApplicationContext::_initialize() {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    QByteArray qDataDir = dataFolder().toUtf8();
    nbl::StringView nblDataDir = nbl::StringView(qDataDir.length(), qDataDir.data());

    NBL_ASSERT(nbl::makeDirectories(nblDataDir).success());

    nbl::StringView path;
    NBL_ASSERT(nbl::StringBuilder(&frame).append(nblDataDir).append("/nebula.db").build(&path).success());

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
            _pushNotification(this, "error", "Cannot start installation", "A process for this installation is already running!");
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
            return nbl::error::ILLEGAL_STATE;
        }

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::Span<nbl::FieldValue> values = {
            .data = nullptr,
            .length = installType->fields.length
        };

        frame.allocate(&values.data, installType->fields.length);

        nbl::JsonValue value = {};
        NBL_CHECK(nbl::parseJson(nbl::StringReader(meta.command), &frame, &value, true));
        NBL_CHECK(nbl::decodeFieldValues(installType, value.object, values));

        NBL_CHECK(installType->launch(&_mContext, values, installationId, handle->handle()));
        NBL_CHECK(handle->initLogging());

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
            return nbl::error::ILLEGAL_STATE;
        }

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::Span<nbl::FieldValue> values = {
            .data = nullptr,
            .length = installType->fields.length
        };

        frame.allocate(&values.data, installType->fields.length);

        nbl::JsonValue value = {};
        NBL_CHECK(nbl::parseJson(nbl::StringReader(meta.command), &frame, &value, true));
        NBL_CHECK(nbl::decodeFieldValues(installType, value.object, values));

        nbl::StringView folder = {};
        NBL_CHECK(installType->getFolder(&_mContext, values, installationId, &folder));

        if(folder.empty()) {
            _pushNotification(this, "error", "Could not open folder", "The installation did not specify a valid installation path");
            return nbl::Result::SUCCESS;
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(fromNbl(folder)));
        return nbl::Result::SUCCESS;
    });

    if(res.error()) {
        _pushNotification(this, "error", "Could not open folder", "An error occurred while getting the installation folder.");
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
            .banner = nbl::StringView(banner.length(), banner.constData()),
            .poster = nbl::StringView(poster.length(), poster.constData()),
            .icon = nbl::StringView(icon.length(), icon.constData()),
            .logo = nbl::StringView(logo.length(), logo.constData()),
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

    NBL_CHECK(nbl::addInstallation(&_mConnection, meta, &meta.id));

    for(QString& qs : params.find("addedInstallationTags")->toStringList()) {
        tagInstallation(meta.id, qs);
    }
    for(QString& qs : params.find("removedInstallationTags")->toStringList()) {
        untagInstallation(meta.id, qs);
    }
    for(QString& qs : params.find("addedGameTags")->toStringList()) {
        tagGame(realGId, qs);
    }
    for(QString& qs : params.find("removedGameTags")->toStringList()) {
        untagGame(realGId, qs);
    }

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

    for(QString& qs : params.find("addedInstallationTags")->toStringList()) {
        tagInstallation(meta.id, qs);
    }
    for(QString& qs : params.find("removedInstallationTags")->toStringList()) {
        untagInstallation(meta.id, qs);
    }
    for(QString& qs : params.find("addedGameTags")->toStringList()) {
        tagGame(meta.id, qs);
    }
    for(QString& qs : params.find("removedGameTags")->toStringList()) {
        untagGame(meta.id, qs);
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
            NBL_CHECK_OR_ROLLBACK(connection, pushImage(icon, "inline_icon", &icon));
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

        QByteArray tagsJson = QJsonDocument(filter["tags"].toJsonArray()).toJson(QJsonDocument::JsonFormat::Compact);
        QByteArray excludedTagsJson = QJsonDocument(filter["excludeTags"].toJsonArray()).toJson(QJsonDocument::JsonFormat::Compact);

        nbl::StringView nblTags = nbl::StringView(tagsJson.length(), tagsJson.data());
        nbl::StringView nblExcludeTags = nbl::StringView(excludedTagsJson.length(), excludedTagsJson.data());

        nbl::JsonValue tags = {};
        NBL_CHECK_OR_ROLLBACK(connection, nbl::parseJson(nbl::StringReader(nblTags), &frame, &tags, true));

        nbl::JsonValue exclude = {};
        NBL_CHECK_OR_ROLLBACK(connection, nbl::parseJson(nbl::StringReader(nblExcludeTags), &frame, &exclude, true));

        nbl::FilterMeta meta = {
            .filterId = filterId,
            .tabId = tabId,
            .priority = priority,
            .name = nbl::StringView(name.length(), name.data()),
            .icon = nbl::StringView(realIcon.length(), realIcon.data()),
            .colorizeIcon = colorizeIcon,
            .multiLine = multiLine,
            .showTitles = showTitles,
            .useIcons = useIcons,
            .criteria = {
                .tags = tags.array,
                .excludedTags = exclude.array,
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
            NBL_CHECK_OR_ROLLBACK(connection, nbl::addFilter(connection, meta, &meta.filterId));
        } else {
            NBL_CHECK_OR_ROLLBACK(connection, nbl::updateFilter(connection, meta));
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
        .name = nbl::StringView(name.length(), name.data()),
        .icon = nbl::StringView(realIcon.length(), realIcon.data()),
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
        .name = nbl::StringView(name.length(), name.data()),
        .icon = nbl::StringView(realIcon.length(), realIcon.data()),
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
            for(const nbl::JsonValue& tagSet : criteria.tags) {
                QList<QString> tagsInSet = {};
                for(const nbl::JsonValue& tag : tagSet.array) {
                    tagsInSet.append(fromNbl(tag.string));
                }
                tags.append(tagsInSet);
            }

            QVariantList excludeTags = {};
            for(const nbl::JsonValue& excluded : criteria.excludedTags) {
                excludeTags.append(fromNbl(excluded.string));
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
int ApplicationContext::addTag(QString name, QColor color) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::addTag(&_mConnection, nblName, color.rgba());
}

int ApplicationContext::removeTag(QString name) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::removeTag(&_mConnection, nblName);
}

int ApplicationContext::renameTag(QString name, QStringView newName) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    QByteArray newArr = newName.toUtf8();
    nbl::StringView nblNewName = nbl::StringView(newArr.length(), newArr.data());
    return nbl::renameTag(&_mConnection, nblName, nblNewName);
}

int ApplicationContext::recolorTag(QString name, QColor color) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::setTagColor(&_mConnection, nblName, color.rgba());
}

int ApplicationContext::addTagParent(QString tag, QString parent) {
    QByteArray arr = tag.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    QByteArray parentArr = parent.toUtf8();
    nbl::StringView nblParent = nbl::StringView(parentArr.length(), parentArr.data());
    return nbl::addParentTag(&_mConnection, nblParent, nblName);
}

int ApplicationContext::tagGame(int64_t gameId, QString name) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::addGameTag(&_mConnection, gameId, nblName);
}

int ApplicationContext::untagGame(int64_t gameId, QString name) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::removeGameTag(&_mConnection, gameId, nblName);
}

int ApplicationContext::tagInstallation(int64_t installationId, QString name) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::addInstallationTag(&_mConnection, installationId, nblName);
}

int ApplicationContext::untagInstallation(int64_t installationId, QString name) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.length(), arr.data());
    return nbl::removeInstallationTag(&_mConnection, installationId, nblName);
}

int ApplicationContext::updateTags(QVariantList modifiedTags) {
    for(QVariant tag : modifiedTags) {

        QVariantMap map = tag.toMap();

        QString originalName = map["originalName"].toString();
        QString name = map["name"].toString();
        QColor color = map["color"].value<QColor>();
        QStringList newParents = map["addedParents"].toStringList();
        QStringList removedParents = map["removedParents"].toStringList();

        NBL_CHECK(_mConnection.beginTransaction());

        if(originalName != name) {
            renameTag(originalName, name);
        }

        recolorTag(name, color);

        QByteArray nameUtf = name.toUtf8();
        nbl::StringView nameView = nbl::StringView(nameUtf.length(), nameUtf.data());

        for(QString parent : newParents) {
            QByteArray parentUtf = parent.toUtf8();
            nbl::StringView parentView = nbl::StringView(parentUtf.length(), parentUtf.data());
            nbl::addParentTag(&_mConnection, parentView, nameView);
        }
        for(QString parent : removedParents) {
            QByteArray parentUtf = parent.toUtf8();
            nbl::StringView parentView = nbl::StringView(parentUtf.length(), parentUtf.data());
            nbl::removeParentTag(&_mConnection, parentView, nameView);
        }

        NBL_CHECK(_mConnection.commitTransaction());
    }

    return 0;
}


QVariantList ApplicationContext::getTagsFromNames(QList<QString> names) {
    QList<QVariant> out = {};

    QByteArray namesJson = QJsonDocument(QJsonArray::fromStringList(names)).toJson(QJsonDocument::JsonFormat::Compact);

    nbl::getTagsFromNames(&_mConnection, nbl::StringView(namesJson.length(), namesJson.data()), [&](nbl::StringView name, u32 color) {
        QMap<QString, QVariant> tag = {};
        tag["name"] = fromNbl(name);
        tag["color"] = QColor(color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });
    return out;
}

QList<QVariant> ApplicationContext::getGameTags(int64_t gameId) {
    QList<QVariant> out = {};
    nbl::getGameTags(&_mConnection, gameId, [&](nbl::StringView name, u32 color) {
        QMap<QString, QVariant> tag = {};
        tag["name"] = fromNbl(name);
        tag["color"] = QColor(color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });
    return out;
}

QList<QVariant> ApplicationContext::getInstallationTags(int64_t installationId) {
    QList<QVariant> out = {};
    nbl::getDirectInstallationTags(&_mConnection, installationId, [&](nbl::StringView name, u32 color) {
        QMap<QString, QVariant> tag = {};
        tag["name"] = fromNbl(name);
        tag["color"] = QColor(color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });

    return out;
}

QList<QVariant> ApplicationContext::getTagParents(QString tag) {
    QList<QVariant> out = {};

    QByteArray tagUtf = tag.toUtf8();
    nbl::StringView tagView = nbl::StringView(tagUtf.length(), tagUtf.data());

    nbl::parents(&_mConnection, tagView, [&](nbl::StringView name, u32 color) {
        QMap<QString, QVariant> tag = {};
        tag["name"] = fromNbl(name);
        tag["color"] = QColor(color);

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
        .banner = nbl::StringView(banner.length(), banner.constData()),
        .poster = nbl::StringView(poster.length(), poster.constData()),
        .icon = nbl::StringView(icon.length(), icon.constData()),
        .logo = nbl::StringView(logo.length(), logo.constData()),
        .logoPosition = position
    };

    NBL_CHECK(nbl::addDisplay(&_mConnection, displayMeta, &dId));

    QByteArray title = params.find("title")->toString().toUtf8();
    QByteArray description = params.find("description")->toString().toUtf8();
    u64 steamId = params.find("steamId")->toUInt();
    u64 igdbId = params.find("igdbId")->toUInt();
    u64 releaseDate = params.find("releaseDate")->toUInt();
    f64 rating = params.find("rating")->toDouble();

    nbl::GameMeta gameMeta = {
        .id = 0,
        .displayId = dId,
        .title = nbl::StringView(title.length(), title.constData()),
        .description = nbl::StringView(description.length(), description.constData()),
        .steamId = steamId,
        .igdbId = igdbId,
        .releaseDate = releaseDate,
        .rating = rating
    };

    i64 gId = 0;
    NBL_CHECK(nbl::addGame(&_mConnection, gameMeta, &gId));

    for(QString& qs : params.find("tags")->toStringList()) {
        tagGame(gId, qs);
    }

    return 0;
}
int ApplicationContext::updateGame(QVariantMap params) {

    bool ok = true;
    i64 gId = params.find("id")->toLongLong(&ok);
    i64 dId = params.find("displayId")->toLongLong(&ok);

    if(!ok) {
        return nbl::error::ILLEGAL_ARGUMENT;
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
        .banner = nbl::StringView(banner.length(), banner.constData()),
        .poster = nbl::StringView(poster.length(), poster.constData()),
        .icon = nbl::StringView(icon.length(), icon.constData()),
        .logo = nbl::StringView(logo.length(), logo.constData()),
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
        .title = nbl::StringView(title.length(), title.constData()),
        .description = nbl::StringView(description.length(), description.constData()),
        .steamId = steamId,
        .igdbId = igdbId,
        .releaseDate = releaseDate,
        .rating = rating
    };

    NBL_CHECK(nbl::updateGame(&_mConnection, gId, gameMeta));

    for(QString& qs : params.find("tags")->toStringList()) {
        tagGame(gId, qs);
    }
    for(QString& qs : params.find("removedTags")->toStringList()) {
        untagGame(gId, qs);
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


