#pragma once

#include <QObject>
#include <QColor>
#include <QtQml>
#include <QProcess>

#include <Data/Database.hpp>
#include <Platform/File.hpp>
#include <Platform/Pool.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/PluginLoader.hpp>

#include "Models/NblInstallationDetails.hpp"
#include "Models/NblGameDetails.hpp"
#include "Models/NblFieldList.hpp"
#include "Models/NblAccountsModel.hpp"
#include "NblProcessHandle.hpp"

inline QByteArray fromNbl(nbl::StringView sv) {
    return QByteArray(sv.data, sv.length);
}


class ApplicationContext : public QObject {

    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Properties
    Q_PROPERTY(QString dataFolder READ dataFolder CONSTANT)
    Q_PROPERTY(qint64 deviceId READ deviceId CONSTANT)

public:

    static ApplicationContext* instance() {
        static ApplicationContext _instance {};
        return &_instance;
    }

    static ApplicationContext* create(QQmlEngine* engine, QJSEngine* scriptEngine) {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);

        return instance();
    }

    QString dataFolder() const;
    qint64 deviceId() const { return _mDeviceId; }

    Q_INVOKABLE NblProcessHandle* launchInstallation(qint64 installationId);
    Q_INVOKABLE NblProcessHandle* getRunningInstallation(qint64 installationId);
    Q_INVOKABLE int openInstallationFolder(qint64 installationId);

    Q_INVOKABLE NblInstallationDetails* getInstallationDetails(qint64 installationId);
    Q_INVOKABLE NblGameDetails* getGameDetails(qint64 gameId);

    Q_INVOKABLE int addInstallation(const QVariantMap& params);
    Q_INVOKABLE int updateInstallation(qint64 id, const QVariantMap& params);
    Q_INVOKABLE int deleteInstallation(qint64 id);

    Q_INVOKABLE int addTab(const QVariantMap& params);
    Q_INVOKABLE int updateTab(qint64 id, const QVariantMap& params);
    Q_INVOKABLE int deleteTab(qint64 id);
    Q_INVOKABLE QVariantMap getTabMeta(qint64 id);

    Q_INVOKABLE qint64 addTag(QString name, QColor color);
    Q_INVOKABLE qint64 findTagId(QString name);
    Q_INVOKABLE int removeTag(qint64 tag);
    Q_INVOKABLE int renameTag(qint64 tag, QStringView newName);
    Q_INVOKABLE int recolorTag(qint64 tag, QColor color);
    Q_INVOKABLE int addTagParent(qint64 tag, qint64 parent);
    Q_INVOKABLE int tagGame(qint64 gameId, qint64 tag);
    Q_INVOKABLE int untagGame(qint64 gameId, qint64 tag);
    Q_INVOKABLE int tagInstallation(qint64 installationId, qint64 tag);
    Q_INVOKABLE int untagInstallation(qint64 installationId, qint64 tag);
    Q_INVOKABLE int updateTags(QVariantList modifiedTags);
    
    Q_INVOKABLE QVariantList getTagsFromIds(QVariantList ids);
    Q_INVOKABLE QList<QVariant> getGameTags(qint64 gId);
    Q_INVOKABLE QList<QVariant> getInstallationTags(qint64 iId);
    Q_INVOKABLE QList<QVariant> getTagParents(qint64 tag);

    Q_INVOKABLE int addGame(QVariantMap details);
    Q_INVOKABLE int updateGame(QVariantMap details);
    Q_INVOKABLE int deleteGame(qint64 gameId);

    Q_INVOKABLE NblFieldList* getInstallationTypeFields(QString installationType);
    Q_INVOKABLE NblAccountsModel* accountsModel(QString accountProvider);

    Q_INVOKABLE void pushNotification(QVariantMap details) {
        emit notification(details);
    }

    Q_INVOKABLE void saveSettings(const QVariantMap& settings);
    Q_INVOKABLE QVariantMap loadSettings();

    Q_INVOKABLE QUrl mediaUrl(QString relativePath, QByteArray repo);

    nbl::DatabaseConnection* databaseConnection() {
        return _mContext.databaseConnection();
    }

    nbl::PluginContext* pluginContext() {
        return &_mContext;
    }

    nbl::PluginLoader* pluginLoader() {
        return &_mLoader;
    }

    void init() {
        if(!_bInitialized) {
            _initialize();
            _bInitialized = true;
        }
    }

signals:

    void notification(QVariantMap details);

private:

    void _initialize();
    ApplicationContext() = default;

    bool _bInitialized = false;
    nbl::PluginContext _mContext;
    nbl::PluginLoader _mLoader;
    QHash<i64, NblProcessHandle*> _mRunningProcesses;

    i64 _mDeviceId;

};
