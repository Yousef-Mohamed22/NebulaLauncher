#pragma once

#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>
#include <QProcess>

#include <Data/Database.hpp>
#include <Platform/File.hpp>
#include <Platform/Pool.hpp>

#include "NblTabRef.hpp"
#include "NblInstallationDetails.hpp"

class NblRunningInstallation : public QObject {

    Q_OBJECT
    QML_ELEMENT

public:

    NblRunningInstallation(QObject* parent = nullptr) : 
        QObject(parent),
        _mProcess(this),
        _mInstallationId(0),
        _mStartTime(0)
    {
        QObject::connect(&_mProcess, &QProcess::finished, this, &NblRunningInstallation::_finished);
    }

    void launch(i64 installationId, nbl::StringView command);

    const QProcess* process() const { return &_mProcess; }

private:

    QProcess _mProcess;
    i64 _mInstallationId;
    u64 _mStartTime;

    void _finished(int exitCode, QProcess::ExitStatus exitStatus);

};


class ApplicationContext : public QObject {

    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Properties
    Q_PROPERTY(QQmlListProperty<NblTabRef> allTabs READ allTabs NOTIFY tabsUpdated)

    Q_PROPERTY(QString dataFolder READ dataFolder CONSTANT)

public:

    static ApplicationContext* instance() {
        static ApplicationContext _instance;
        return &_instance;
    }

    static ApplicationContext* create(QQmlEngine* engine, QJSEngine* scriptEngine) {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);

        ApplicationContext* context = instance();

        QQmlEngine::setObjectOwnership(context, QQmlEngine::CppOwnership);

        NBL_ASSERT(nbl::Arena::create(1024 * 1024 * 10, &context->_mProcessArena).success());
        NBL_ASSERT(nbl::PoolView<NblRunningInstallation>::create(
            &context->_mProcessArena,
            &context->_mRunningInstallations
        ).success());
        

        return context;
    }

    QQmlListProperty<NblTabRef> allTabs() { 
        return QQmlListProperty<NblTabRef>(
            this,
            &_mAllTabs,
            [](QQmlListProperty<NblTabRef>* ref) -> qsizetype {
                QList<NblTabRef>* list = static_cast<QList<NblTabRef>*>(ref->data);
                return list->count();
            },
            [](QQmlListProperty<NblTabRef>* ref, qsizetype at) -> NblTabRef* {
                QList<NblTabRef>* list = static_cast<QList<NblTabRef>*>(ref->data);
                if(at > list->count()) return nullptr;
                return list->data() + at;
            }
        );
    }


    QString dataFolder() const {
        nbl::StringView dataDir = nbl::getDataDirectory();
        return QString::fromUtf8(dataDir.data, dataDir.length).append("/NebulaLauncher");
    }

    // Functions
    Q_INVOKABLE i32 reload() {
        return _reload().value;
    }

    Q_INVOKABLE void launchInstallation(i64 installationId);

    Q_INVOKABLE NblInstallationDetails* getInstallationDetails(i64 installationId);

    Q_INVOKABLE int addInstallation(const QVariantMap& params);
    
    Q_INVOKABLE int updateInstallation(int64_t id, const QVariantMap& params);

    Q_INVOKABLE void saveSettings(const QVariantMap& settings);

    Q_INVOKABLE QVariantMap loadSettings();

    nbl::DatabaseConnection* connection() {
        return &_mConnection;
    }

signals:

    void tabsUpdated();

private:

    nbl::Result _reload();

    ApplicationContext() = default;

    nbl::DatabaseConnection _mConnection;
    QList<NblTabRef> _mAllTabs;

    nbl::Arena _mProcessArena;
    nbl::PoolView<NblRunningInstallation> _mRunningInstallations;

    friend class NblRunningInstallation;

};
