#include "Game/GameInstallationsModel.hpp"
#include "ApplicationContext.hpp"

#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant nbl::frontend::GameInstallationsModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    const InstallationRef& ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.installationId());
        case TitleRole:
            return ref.title();
        case GameTitleRole:
            return ref.gameTitle();
        case PosterRole:
            return nbl::frontend::mediaUrl(ref.poster());
        case IconRole:
            return nbl::frontend::mediaUrl(ref.icon());
        case LastPlayedRole:
            return static_cast<quint64>(ref.lastPlayed());
        case ResolvedTitleRole:
            return ref.resolvedTitle();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::GameInstallationsModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "installationId";
    roles[TitleRole] = "title";
    roles[GameTitleRole] = "gameTitle";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    roles[LastPlayedRole] = "lastPlayed";
    roles[ResolvedTitleRole] = "resolvedTitle";
    return roles;
}

nbl::Result nbl::frontend::GameInstallationsModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<InstallationRef> found = {};
    found.reserve(limit);

    NBL_CHECK(nbl::getInstallationsOfGame(db, _mGameId, ApplicationContext::instance()->deviceId(), limit, offset, [&](const nbl::InstallationLookupResult& res) -> nbl::Result {
        InstallationRef ref(res.installationId,
            fromNbl(res.title),
            fromNbl(res.gameTitle),
            res.poster,
            res.icon,
            res.lastPlayed
        );
        found.append(ref);
        return nbl::Result::SUCCESS;
    }));

    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
        emit countChanged();
    }

    return nbl::Result::SUCCESS;
}
