#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/AccountProvider.hpp>

#include <QObject>
#include <QtQml>

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
        IconRole,
        IconNearestScalingRole,
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
            case IconNearestScalingRole:
                return _mData[index.row()].nearest;
            default:
                return QVariant();
        }
    }

    inline QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = {};
        roles[IdRole] = "id";
        roles[NameRole] = "name";
        roles[IconRole] = "icon";
        roles[IconNearestScalingRole] = "iconNearestScaling";
        return roles;
    }

    Q_INVOKABLE void refresh();
 
private:

    struct _AccountInfo {
        QString name;
        QString icon;
        quint32 id;
        bool nearest;
    };

    const nbl::AccountProvider* _pProvider;
    QList<_AccountInfo> _mData;

};
