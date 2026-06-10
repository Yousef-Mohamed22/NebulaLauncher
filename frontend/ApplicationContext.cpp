#include "ApplicationContext.hpp"
#include "NblInstallationDetails.hpp"
#include "Platform/Arena.hpp"
#include "Platform/File.hpp"

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Time.hpp>
#include <Data/Model.hpp>

#include <QString>
#include <qcontainerfwd.h>
#include <qprocess.h>

#include <openssl/evp.h>


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




void NblRunningInstallation::launch(i64 installationId, nbl::StringView command) {

    _mInstallationId = installationId;
    _mStartTime = nbl::getCurrentTime();

    nbl::log(nbl::LogLevel::INFO, "Launching game with command: ", command);

    QString qCommand = QString::fromUtf8(command.data, command.length);

    _mProcess.startCommand(qCommand);

}

void NblRunningInstallation::_finished(int exitCode, QProcess::ExitStatus exitStatus) {

    nbl::LogLevel lvl = exitCode == 0 ? nbl::LogLevel::INFO : nbl::LogLevel::ERROR;
    nbl::log(lvl, "Subprocess exited with code ", exitCode);

    ApplicationContext* context = ApplicationContext::instance();
    nbl::DatabaseConnection* connection = context->connection();

    u64 playTime = nbl::getCurrentTime() - _mStartTime;
    nbl::bumpInstallationPlayTime(connection, _mInstallationId, playTime);

    context->_mRunningInstallations.pop(this);

}

nbl::Result ApplicationContext::_reload() {

    if(!_mConnection.isValid()) {

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::StringView path;

        NBL_CHECK(nbl::StringBuilder(&frame).append(nbl::getDataDirectory()).append("/NebulaLauncher/nebula.db").build(&path));

        NBL_CHECK(nbl::DatabaseConnection::open(path, &_mConnection));
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

    nbl::getInstallationCommand(&_mConnection, installationId, [](void* userData, const nbl::InstallationMeta& meta) {

        ApplicationContext* context = static_cast<ApplicationContext*>(userData);

        NblRunningInstallation* runningInstallation;
        NBL_CHECK(context->_mRunningInstallations.push(&runningInstallation));

        runningInstallation->launch(meta.id, meta.command);

        return nbl::Result::SUCCESS;
    }, this);

    nbl::markInstallationJustPlayed(&_mConnection, installationId);
}

NblInstallationDetails* ApplicationContext::getInstallationDetails(i64 installationId) {

    if(!_mConnection.isValid()) {
        nbl::log(nbl::LogLevel::ERROR, "Cannot get installation details, database connection is not valid");
        return nullptr;
    }

    NblInstallationDetails* details;

    if(nbl::getInstallationMeta(&_mConnection, installationId, [](void* userData, const nbl::InstallationMeta& meta) {

         *static_cast<NblInstallationDetails**>(userData) = new NblInstallationDetails(
            meta.id,
            QString::fromUtf8(meta.title.data, meta.title.length),
            QString::fromUtf8(meta.description.data, meta.description.length),
            QString::fromUtf8(meta.poster.data, meta.poster.length),
            QString::fromUtf8(meta.banner.data, meta.banner.length),
            QString::fromUtf8(meta.icon.data, meta.icon.length),
            QString::fromUtf8(meta.logo.data, meta.logo.length),
            static_cast<u64>(meta.logoPosition),
            meta.playTime,
            meta.playCount,
            meta.lastPlayed,
            meta.added,
            meta.releaseDate,
            meta.rating,
            QString::fromUtf8(meta.gameTitle.data, meta.gameTitle.length),
            QString::fromUtf8(meta.gamePoster.data, meta.gamePoster.length)
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

        QByteArray title = params.find("title")->toString().toUtf8();
        QByteArray description = params.find("description")->toString().toUtf8();

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
            .title = nbl::StringView(title.length(), title.constData()),
            .description = nbl::StringView(description.length(), description.constData()),
            .banner = nbl::StringView(banner.length(), banner.constData()),
            .poster = nbl::StringView(poster.length(), poster.constData()),
            .icon = nbl::StringView(icon.length(), icon.constData()),
            .logo = nbl::StringView(logo.length(), logo.constData()),
            .logoPosition = position
        };

        NBL_CHECK(nbl::addDisplay(&_mConnection, meta, &dId));

    } else {
        // Get display id from game
        NBL_CHECK(nbl::getGameMeta(&_mConnection, gId, [](void* userData, const nbl::GameMeta& meta) {
            *static_cast<i64*>(userData) = meta.dId;
            return nbl::Result::SUCCESS;
        }, &dId));
    }

    if(gId == 0) {
        // Add a new game
        u64 steamId = params.find("steamId")->toUInt();
        u64 igdbId = params.find("igdbId")->toUInt();
        u64 releaseDate = params.find("releaseDate")->toUInt();
        f64 rating = params.find("rating")->toDouble();

        nbl::GameMeta meta = {
            .igdbId = igdbId,
            .steamId = steamId,
            .dId = dId,
            .releaseDate = releaseDate,
            .rating = rating
        };

        NBL_CHECK(nbl::addGame(&_mConnection, meta, &gId));
    }

    // Add a new installation
    QByteArray command = params.find("command")->toString().toUtf8();
    NBL_CHECK(nbl::addInstallation(&_mConnection, gId, nbl::StringView(command.length(), command.data()), nullptr));

    NBL_CHECK(_mConnection.commitTransaction());

    return 0;
}
