#include "Tag/TagSearchModel.hpp"
#include "ApplicationContext.hpp"
#include "Core/Result.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant nbl::frontend::TagSearchModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    TagRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.id;
        case NameRole:
            return ref.name;
        case ColorRole:
            return ref.color;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::TagSearchModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


nbl::Result nbl::frontend::TagSearchModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    nbl::StringView query = nbl::StringView(_mSearchQuery.data(), _mSearchQuery.length());

    QList<TagRef> found;
    bool exactMatch;
    found.reserve(limit);

    if(query.empty()) {
        NBL_CHECK(nbl::enumerateTags(db, limit, offset, [&](nbl::TagMeta tag) -> nbl::Result {

            TagRef ref(tag.id, QString::fromUtf8(tag.name.data, tag.name.length), QColor::fromRgb(tag.color)); 
            found.append(ref);
            return nbl::Result::SUCCESS;

        }));
    } else {
        NBL_CHECK(nbl::searchTags(db, query, limit, offset, [&](nbl::TagMeta tag) -> nbl::Result {

            if(query == tag.name) {
                exactMatch = true;
            }

            TagRef ref(tag.id, QString::fromUtf8(tag.name.data, tag.name.length), QColor::fromRgb(tag.color)); 
            found.append(ref);
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

    if(exactMatch && !_bExactMatch) {
        _bExactMatch = true;
        exactMatchChanged();
    }

    return nbl::Result::SUCCESS;
}
