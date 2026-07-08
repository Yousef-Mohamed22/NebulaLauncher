#pragma once

#include <qcontainerfwd.h>
#include <qqmlintegration.h>
#include <qstringview.h>
#include <qtenvironmentvariables.h>
#include <qtmetamacros.h>
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

    Q_INVOKABLE NblProcessHandle* launchInstallation(i64 installationId);
    Q_INVOKABLE NblProcessHandle* getRunningInstallation(i64 installationId);

    Q_INVOKABLE NblInstallationDetails* getInstallationDetails(i64 installationId);

    Q_INVOKABLE int addInstallation(const QVariantMap& params);
    Q_INVOKABLE int updateInstallation(int64_t id, const QVariantMap& params);
    Q_INVOKABLE int deleteInstallation(int64_t id);

    Q_INVOKABLE int addTab(const QVariantMap& params);
    Q_INVOKABLE int updateTab(qint64 id, const QVariantMap& params);
    Q_INVOKABLE int deleteTab(qint64 id);
    Q_INVOKABLE QVariantMap getTabMeta(qint64 id);

    Q_INVOKABLE int addTag(QString name, QColor color);
    Q_INVOKABLE int removeTag(QString name);
    Q_INVOKABLE int renameTag(QString name, QStringView newName);
    Q_INVOKABLE int recolorTag(QString name, QColor color);
    Q_INVOKABLE int tagGame(int64_t gameId, QString name);
    Q_INVOKABLE int untagGame(int64_t gameId, QString name);
    Q_INVOKABLE int tagInstallation(int64_t installationId, QString name);
    Q_INVOKABLE int untagInstallation(int64_t installationId, QString name);

    Q_INVOKABLE QVariantList getTagsFromNames(QList<QString> names);
    Q_INVOKABLE QList<QVariant> getGameTags(int64_t gId);
    Q_INVOKABLE QList<QVariant> getInstallationTags(int64_t iId);

    Q_INVOKABLE void saveSettings(const QVariantMap& settings);
    Q_INVOKABLE QVariantMap loadSettings();

    nbl::DatabaseConnection* connection() {
        return &_mConnection;
    }

    nbl::PluginContext* pluginContext() {
        return &_mContext;
    }

    void init() {
        if(!_bInitialized) {
            _initialize();
            _bInitialized = true;
        }
    }

private:

    void _initialize();
    ApplicationContext() = default;

    bool _bInitialized = false;
    nbl::DatabaseConnection _mConnection;
    nbl::PluginContext _mContext;
    nbl::PluginLoader _mLoader;

    friend class NblRunningInstallation;

};
