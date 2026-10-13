/**
 * \file TabFunctions.cpp
 * \author jwall64
 * \brief Partial implementation of ApplicationContext.hpp, containing functions related to managing tabs
 */


#include "ApplicationContext.hpp"

namespace {

nbl::Result _pushFilters(nbl::DatabaseConnection* connection, i64 tabId, const QVariantList& filters, std::function_ref<nbl::Result(QString, QString, QString*)> imageHandler) {

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
        bool onlyInstalled = filter["onlyInstalled"].toBool();
        bool showInstallations = filter["showInstallations"].toBool();
        bool showFilterInfo = filter["showFilterInfo"].toBool();
        i64 priority = filter["priority"].toLongLong();

        if(icon.endsWith(".png") || icon.endsWith(".svg")) {
            NBL_CHECK(imageHandler(icon, "inline_icon", &icon));
        }
        QByteArray realIcon = icon.toUtf8();

        nbl::SortBy sortBy = static_cast<nbl::SortBy>(filter["sortBy"].toInt());
        nbl::SortOrder sortOrder = static_cast<nbl::SortOrder>(filter["sortOrder"].toInt());
        
        u64 minReleaseDate = filter["minReleaseDate"].toULongLong();
        u64 maxReleaseDate = filter["minReleaseDate"].toULongLong();
        f64 minRating = filter["minRating"].toDouble();
        f64 maxRating = filter["minRating"].toDouble();
        u64 minPlayTime = filter["minTime"].toULongLong();
        u64 maxPlayTime = filter["minTime"].toULongLong();
        u64 minAchievements = filter["minAchievements"].toULongLong();
        u64 maxAchievements = filter["maxAchievements"].toULongLong();
        u64 minUnlockedAchievements = filter["minUnlockedAchievements"].toULongLong();
        u64 maxUnlockedAchievements = filter["maxUnlockedAchievements"].toULongLong();


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


        nbl::FilterFlags flags;
        if(colorizeIcon) flags |= nbl::FilterFlag::COLORIZE_ICON;
        if(multiLine) flags |= nbl::FilterFlag::MULTI_LINE;
        if(showTitles) flags |= nbl::FilterFlag::SHOW_TITLES;
        if(useIcons) flags |= nbl::FilterFlag::USE_ICONS;
        if(onlyInstalled) flags |= nbl::FilterFlag::ONLY_INSTALLED;
        if(showInstallations) flags |= nbl::FilterFlag::SHOW_INSTALLATIONS;
        if(showFilterInfo) flags |= nbl::FilterFlag::SHOW_FILTER_INFO;


        nbl::FilterMeta meta = {
            .filterId = filterId,
            .tabId = tabId,
            .priority = priority,
            .name = nbl::StringView(name.data(), name.length()),
            .icon = nbl::StringView(realIcon.data(), realIcon.length()),
            .flags = flags,
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
                .sortOrder = sortOrder,
                .minAchievements = minAchievements,
                .maxAchievements = maxAchievements,
                .minUnlockedAchievements = minUnlockedAchievements,
                .maxUnlockedAchievements = maxUnlockedAchievements
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

    if(icon.endsWith(".png") || icon.endsWith(".svg")) {
        NBL_CHECK(_pushImage(icon, "inline_icon", &icon));
    }

    QByteArray realIcon = icon.toUtf8();

    nbl::TabFlags flags;
    if(colorizeIcon) flags |= nbl::TabFlag::COLORIZE_ICON;
    if(hidden) flags |= nbl::TabFlag::HIDDEN;

    nbl::TabMeta meta = {
        .name = nbl::StringView(name.data(), name.length()),
        .icon = nbl::StringView(realIcon.data(), realIcon.length()),
        .flags = flags
    };

    nbl::DatabaseConnection* c = databaseConnection();
    NBL_CHECK(c->beginTransaction());

    NBL_CHECK_OR_ROLLBACK(c, nbl::addTab(c, meta, &meta.tabId));
    NBL_CHECK_OR_ROLLBACK(c, _pushFilters(c, meta.tabId, params["filters"].toList(), [&](QString icon, QString repo, QString* out) { return _pushImage(icon, repo, out); }));

    NBL_CHECK(c->commitTransaction());

    return nbl::Result::SUCCESS;
}
int ApplicationContext::updateTab(qint64 id, const QVariantMap& params) {

    QByteArray name = params["name"].toByteArray();
    QString icon = params["icon"].toString();
    bool colorizeIcon = params["colorizeIcon"].toBool();
    bool hidden = params["hidden"].toBool();

    if(icon.endsWith(".png") || icon.endsWith(".svg")) {
        NBL_CHECK(_pushImage(icon, "inline_icon", &icon));
    }

    nbl::TabFlags flags;
    if(colorizeIcon) flags |= nbl::TabFlag::COLORIZE_ICON;
    if(hidden) flags |= nbl::TabFlag::HIDDEN;

    QByteArray realIcon = icon.toUtf8();

    nbl::TabMeta meta = {
        .tabId = static_cast<i64>(id),
        .name = nbl::StringView(name.data(), name.length()),
        .icon = nbl::StringView(realIcon.data(), realIcon.length()),
        .flags = flags
    };

    nbl::DatabaseConnection* c = databaseConnection();
    NBL_CHECK(c->beginTransaction());
    NBL_CHECK_OR_ROLLBACK(c, nbl::updateTab(c, meta));

    NBL_CHECK_OR_ROLLBACK(c, _pushFilters(c, meta.tabId, params["filters"].toList(), [&](QString icon, QString repo, QString* out) { return _pushImage(icon, repo, out); }));

    for(const QVariant& removed : params["removedFilters"].toList()) {
        NBL_CHECK_OR_ROLLBACK(c, nbl::removeFilter(c, removed.toLongLong()));
    }

    NBL_CHECK(c->commitTransaction());
    return nbl::Result::SUCCESS;
}

int ApplicationContext::deleteTab(qint64 id) { 
    return nbl::removeTab(databaseConnection(), id);
}

nbl::frontend::TabRef* ApplicationContext::getTabMeta(qint64 id) {
    return nbl::frontend::TabRef::lookup(id);
}
