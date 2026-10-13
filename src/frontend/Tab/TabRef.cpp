#include "TabRef.hpp"
#include "ApplicationContext.hpp"

#include <Data/Database.hpp>
#include <Data/Model.hpp>

nbl::frontend::TabRef::TabRef(const nbl::TabMeta& meta, QObject* parent) :
    QObject(parent),
    _mName(fromNbl(meta.name)),
    _mIcon(fromNbl(meta.icon)),
    _mTabId(meta.tabId),
    _mFlags(meta.flags)
{ }


QVariantList nbl::frontend::TabRef::encodeFilters() {

    nbl::DatabaseConnection* conn = ApplicationContext::instance()->databaseConnection();
    QVariantList out = {};

    nbl::getFiltersForTab(conn, _mTabId, 0, INT32_MAX, [&](const nbl::FilterMeta& meta) -> nbl::Result {

        QVariantMap filter = {};

        filter["id"] = static_cast<quint64>(meta.filterId);
        filter["name"] = fromNbl(meta.name);
        filter["icon"] = fromNbl(meta.icon);
        
        // Flags
        filter["colorizeIcon"] = (meta.flags & nbl::FilterFlag::COLORIZE_ICON) > 0;
        filter["multiLine"] = (meta.flags & nbl::FilterFlag::MULTI_LINE) > 0;
        filter["useIcons"] = (meta.flags & nbl::FilterFlag::USE_ICONS) > 0;
        filter["showTitles"] = (meta.flags & nbl::FilterFlag::SHOW_TITLES) > 0;
        filter["onlyInstalled"] = (meta.flags & nbl::FilterFlag::ONLY_INSTALLED) > 0;
        filter["showInstallations"] = (meta.flags & nbl::FilterFlag::SHOW_INSTALLATIONS) > 0;
        filter["showFilterInfo"] = (meta.flags & nbl::FilterFlag::SHOW_FILTER_INFO) > 0;
        
        // Criteria
        nbl::getFilterCriteria(conn, meta.filterId, [&](const nbl::SearchCriteria& criteria) -> nbl::Result {

            filter["sortBy"] = static_cast<qint32>(criteria.sortBy);
            filter["sortOrder"] = static_cast<qint32>(criteria.sortOrder);
            filter["minReleaseDate"] = static_cast<quint64>(criteria.minReleaseDate);
            filter["maxReleaseDate"] = static_cast<quint64>(criteria.maxReleaseDate);
            filter["minRating"] = criteria.minRating;
            filter["maxRating"] = criteria.maxRating;
            filter["minTime"] = static_cast<quint64>(criteria.minPlayTime);
            filter["maxTime"] = static_cast<quint64>(criteria.maxPlayTime);
            filter["minAchievements"] = static_cast<quint64>(criteria.minAchievements);
            filter["maxAchievements"] = static_cast<quint64>(criteria.maxAchievements);
            filter["minUnlockedAchievements"] = static_cast<quint64>(criteria.minUnlockedAchievements);
            filter["maxUnlockedAchievements"] = static_cast<quint64>(criteria.maxUnlockedAchievements);

            QVariantList tags{};
            for(nbl::ConstSpan<i64> tagSet : criteria.tags) {
                QVariantList tagsInSet{};
                for(i64 tag : tagSet) {
                    tagsInSet.append(static_cast<qint64>(tag));
                }
                tags.append(QVariant(tagsInSet));
            }

            QVariantList excludeTags{};
            for(i64 tag : criteria.excludedTags) {
                excludeTags.append(static_cast<qint64>(tag));
            }

            filter["tags"] = tags;
            filter["excludeTags"] = excludeTags;

            return nbl::Result::SUCCESS;
        });

        out.append(filter);
        return nbl::Result::SUCCESS;
    });

    return out;
}

nbl::frontend::TabRef* nbl::frontend::TabRef::lookup(qint64 id) {

    nbl::frontend::TabRef* out = nullptr;
    if(nbl::getTabMeta(ApplicationContext::instance()->databaseConnection(), static_cast<i64>(id), [&](const nbl::TabMeta& meta) -> nbl::Result {
        out = new nbl::frontend::TabRef(meta);
        return nbl::Result::SUCCESS;
    }).error()) {
        delete out;
        return nullptr;
    }

    return out;
}
