#pragma once

#include <qtmetamacros.h>
#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/AccountProvider.hpp>

#include <QObject>
#include <QtQml>

#include "ApplicationContext.hpp"

class NblAccountsModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

public:

    NblAccountsModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pProvider(nullptr),
        _mData{}
    { }
        

    NblAccountsModel(const nbl::AccountProvider* provider, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pProvider(provider),
        _mData{}
    { 
        refresh();
    }

    ~NblAccountsModel() = default;

    NblAccountsModel(const NblAccountsModel& other) :
        QAbstractListModel(other.parent()),
        _pProvider(other._pProvider),
        _mData(other._mData)
    { 
        refresh();
    }

    NblAccountsModel(NblAccountsModel&& other) :
        QAbstractListModel(other.parent()),
        _pProvider(other._pProvider),
        _mData(std::move(other._mData))
    { 
    }

    NblAccountsModel& operator=(const NblAccountsModel& other) {
        setParent(other.parent());
        _pProvider = other._pProvider;
        _mData = other._mData;
        return *this;
    }
    
    NblAccountsModel& operator=(NblAccountsModel&& other) {
        if(&other == this) return *this;
        setParent(other.parent());
        _pProvider = other._pProvider;
        _mData = std::move(other._mData);
        return *this;
    }

    enum Roles {
        IdRole,
        NameRole,
        IconRole
    };

    inline int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.length();
    }

    inline QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if(index.row() >= rowCount()) return QVariant();
        
        switch(role) {
            case IdRole:
                return _mData[index.row()].id;
            case NameRole:
                return _mData[index.row()].name;
            case IconRole:
                return _mData[index.row()].icon;
            default:
                return QVariant();
        }
    }

    inline QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = {};
        roles[IdRole] = "id";
        roles[NameRole] = "name";
        roles[IconRole] = "icon";
        return roles;
    }

    Q_INVOKABLE void refresh() {

        beginResetModel();
        _mData.clear();

        if(_pProvider) {
            _pProvider->enumerateAccountInfo([&](const nbl::AccountInfo& info) -> nbl::Result {
                _mData.append({fromNbl(info.userName), fromNbl(info.icon), info.id});
                return nbl::Result::SUCCESS;
            });
        }

        endResetModel();
    } 
 
private:

    struct _AccountInfo {
        QString name;
        QString icon;
        quint32 id;
    };

    const nbl::AccountProvider* _pProvider;
    QList<_AccountInfo> _mData;

};
