#include "ApplicationContext.hpp"

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Time.hpp>
#include <Data/Model.hpp>

#include <QString>
#include <qprocess.h>

void NblRunningInstallation::launch(i64 installationId, nbl::StringView command) {

    _mInstallationId = installationId;
    _mStartTime = nbl::getCurrentTime();

    QString qCommand = QString::fromUtf8(command.data, command.length);
    _mProcess.startCommand(qCommand);

    QObject::connect(&_mProcess, &QProcess::finished, this, &NblRunningInstallation::_finished);
}

void NblRunningInstallation::_finished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    ApplicationContext* context = ApplicationContext::instance();
    nbl::DatabaseConnection* connection = context->connection();

    u64 playTime = nbl::getCurrentTime() - _mStartTime;
    nbl::bumpInstallationPlayTime(connection, _mInstallationId, playTime);

    context->_mRunningInstallations.pop(this);

}

nbl::Result ApplicationContext::_reload() {

    if(!_mConnection.isValid()) {

        NBL_CHECK(nbl::DatabaseConnection::open("nebula.db", &_mConnection));
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
