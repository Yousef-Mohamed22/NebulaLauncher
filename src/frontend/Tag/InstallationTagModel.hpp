#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "Tag/TagRef.hpp"
#include "PagedListModel.hpp"

namespace nbl::frontend {

class InstallationTagModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 installationId READ installationId WRITE setInstallationId NOTIFY installationIdChanged)

public:

    explicit InstallationTagModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mInstallationId(0)
    { }

    InstallationTagModel(i64 installationId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mInstallationId(installationId)
    { }

    enum Roles {
        IdRole,
        NameRole,
        ColorRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 installationId() const {
        return _mInstallationId;
    }

    void setInstallationId(qint64 id) {
        if(id != _mInstallationId) {
            _mInstallationId = id;
            emit installationIdChanged();
            refresh();
        }
    }

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

signals:

    void installationIdChanged();

private:

    void _fetchPageData();
    qint64 _mInstallationId;
    QList<TagRef> _mData;

};

}
