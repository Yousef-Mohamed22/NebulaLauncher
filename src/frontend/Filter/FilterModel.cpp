#include "Filter/FilterModel.hpp"
#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>
#include <Core/Result.hpp>
#include <Platform/Time.hpp>

QVariant nbl::frontend::FilterModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    _Item item = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(item.id);
        case TitleRole:
            return item.title;
        case PosterRole:
            return nbl::frontend::mediaUrl(item.poster);
        case IconRole:
            return nbl::frontend::mediaUrl(item.icon);
        case FilterInfoRole:
            return item.filterInfo;
        case FilterInfoIconRole:
            return item.filterInfoIcon;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::FilterModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    roles[FilterInfoRole] = "filterInfo";
    roles[FilterInfoIconRole] = "filterInfoIcon";
    return roles;
}


nbl::Result nbl::frontend::FilterModel::_fetchPageData(qsizetype limit, qsizetype offset) {


    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<_Item> found = {};
    found.reserve(limit);
    
    if(_mSearchInstallations) {
        NBL_CHECK(nbl::lookupInstallationsForFilter(db, _mFilterId, ApplicationContext::instance()->deviceId(), offset, limit, [&](const nbl::InstallationLookupResult& res) -> nbl::Result {

            QString filterInfoIcon{};
            QString filterInfo{};

            if(res.filter.flags & nbl::FilterFlag::SHOW_FILTER_INFO) {
                switch(res.filter.criteria.sortBy) {
                    default:
                    case nbl::SortBy::TITLE:
                        break;
                    case nbl::SortBy::LAST_PLAYED: {

                        filterInfo = "Played";
                        u64 time = nbl::getCurrentTime();
                        u64 timeSincePlayed = time - res.lastPlayed;

                        if(timeSincePlayed < 1000 * 60 * 60) {
                            u64 minutes = (timeSincePlayed / 1000 / 60);
                            if(minutes == 1) {
                                filterInfo = "Played 1 minute ago";
                            } else {
                                filterInfo = QString("Played %1 minutes ago").arg(minutes);
                            }
                        } else if(timeSincePlayed < 1000 * 60 * 60 * 24) {
                            u64 hours = (timeSincePlayed / 1000 / 60 / 60);
                            if(hours == 1) {
                                filterInfo = "Played 1 hour ago";
                            } else {
                                filterInfo = QString("Played %1 hours ago").arg(hours);
                            }
                        } else {
                            filterInfo = QString("Played on %1")
                                .arg(QDateTime::fromMSecsSinceEpoch(res.lastPlayed).toString("MMM d, yyyy"));
                        }
                        break;
                    }
                    case nbl::SortBy::ADDED: {
                        filterInfo = QString("Added on %1")
                                .arg(QDateTime::fromMSecsSinceEpoch(res.added).toString("MMM d, yyyy"));
                        break;
                    }
                    case nbl::SortBy::PLAY_COUNT: {
                        filterInfo = QString("Played %1 times")
                                .arg(res.playCount);
                        break;
                    }
                    case nbl::SortBy::PLAY_TIME: {
                        u64 hours = (res.playTime / 1000 / 60 / 60);
                        if(hours == 1) {
                            filterInfo = "1 hour played";
                        } else {
                            filterInfo = QString("%1 hours played").arg(hours);
                        }
                        break;
                    }
                    case nbl::SortBy::RELEASE_DATE: {
                        filterInfo = QString("%1")
                            .arg(QDateTime::fromMSecsSinceEpoch(res.releaseDate).toString("MMM d, yyyy"));
                        break;
                    }
                    case nbl::SortBy::RATING: {
                        filterInfoIcon = "";
                        filterInfo = QString("%1%").arg(QString::number(res.rating, 'f', 1));
                        break;
                    }
                    case nbl::SortBy::ACHIEVEMENT_PROGRESS: {

                        u64 unlocked;
                        nbl::countUnlockedGameAchievements(db, res.gameId, &unlocked);
                        u64 total;
                        nbl::countGameAchievements(db, res.gameId, &total);
                        
                        f64 percent = total == 0 ? 0.0 : unlocked / static_cast<f64>(total);

                        filterInfoIcon = "";
                        filterInfo = QString("%1 / %2 (%3%)")
                            .arg(static_cast<quint64>(unlocked))
                            .arg(static_cast<quint64>(total))
                            .arg(QString::number(percent * 100.0, 'f', 1));
                        break;
                    }
                }
            }
            _Item item(
                res.installationId,
                fromNbl(res.title),
                res.poster,
                res.icon,
                filterInfo,
                filterInfoIcon
            );
            found.append(item);
            return nbl::Result::SUCCESS;
        }));    
    } else {
        NBL_CHECK(nbl::lookupGamesForFilter(db, _mFilterId, ApplicationContext::instance()->deviceId(), offset, limit, [&](const nbl::GameLookupResult& res) -> nbl::Result {

            QString filterInfoIcon{};
            QString filterInfo{};

            if(res.filter.flags & nbl::FilterFlag::SHOW_FILTER_INFO) {
                switch(res.filter.criteria.sortBy) {
                    default:
                    case nbl::SortBy::TITLE:
                        break;
                    case nbl::SortBy::LAST_PLAYED: {

                        filterInfo = "Played";
                        u64 time = nbl::getCurrentTime();

                        u64 timeSincePlayed = time - res.lastPlayed;

                        if(timeSincePlayed < 1000 * 60 * 60) {
                            u64 minutes = (timeSincePlayed / 1000 / 60);
                            if(minutes == 1) {
                                filterInfo = "Played 1 minute ago";
                            } else {
                                filterInfo = QString("Played %1 minutes ago").arg(minutes);
                            }
                        } else if(timeSincePlayed < 1000 * 60 * 60 * 24) {
                            u64 hours = (timeSincePlayed / 1000 / 60 / 60);
                            if(hours == 1) {
                                filterInfo = "Played 1 hour ago";
                            } else {
                                filterInfo = QString("Played %1 hours ago").arg(hours);
                            }
                        } else {
                            filterInfo = QString("Played on %1")
                                .arg(QDateTime::fromMSecsSinceEpoch(res.lastPlayed).toString("MMM d, yyyy"));
                        }
                        break;
                    }
                    case nbl::SortBy::ADDED: {
                        u64 added;
                        filterInfo = QString("Added on %1")
                                .arg(QDateTime::fromMSecsSinceEpoch(res.firstAdded).toString("MMM d, yyyy"));
                        break;
                    }
                    case nbl::SortBy::PLAY_COUNT: {
                        if(res.playCount == 1) {
                            filterInfo = "Played 1 time";
                        } else {
                            filterInfo = QString("Played %1 times")
                                .arg(res.playCount);
                        }
                        break;
                    }
                    case nbl::SortBy::PLAY_TIME: {
                        u64 hours = (res.playTime / 1000 / 60 / 60);
                        if(hours == 1) {
                            filterInfo = "1 hour played";
                        } else {
                            filterInfo = QString("%1 hours played").arg(hours);
                        }
                        break;
                    }
                    case nbl::SortBy::RELEASE_DATE: {
                        filterInfo = QString("%1")
                            .arg(QDateTime::fromMSecsSinceEpoch(res.releaseDate).toString("MMM d, yyyy"));
                        break;
                    }
                    case nbl::SortBy::RATING: {
                        filterInfoIcon = "";
                        filterInfo = QString("%1%").arg(QString::number(res.rating, 'f', 1));
                        break;
                    }
                    case nbl::SortBy::ACHIEVEMENT_PROGRESS: {

                        u64 unlocked = res.numUnlockedAchievements;
                        u64 total = res.numAchievements;
                        
                        f64 percent = total == 0 ? 0.0 : unlocked / static_cast<f64>(total);


                        filterInfoIcon = "";
                        filterInfo = QString("%1 / %2 (%3%)")
                            .arg(static_cast<quint64>(unlocked))
                            .arg(static_cast<quint64>(total))
                            .arg(QString::number(percent * 100.0, 'f', 1));
                        break;
                    }
                }
            }

            _Item item(
                res.id,
                fromNbl(res.title),
                res.poster,
                res.icon,
                filterInfo,
                filterInfoIcon
            );
            found.append(item);
            return nbl::Result::SUCCESS;
        }));   
    }

    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
    }
    return nbl::Result::SUCCESS;
}
