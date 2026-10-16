#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <QNetworkReply>
#include <QThreadPool>

#include <Data/MediaManager.hpp>

namespace nbl::frontend {


class MediaImageResponse : public QQuickImageResponse, public QRunnable {

    Q_OBJECT

public:

    MediaImageResponse(qint64 refId, const QSize& requestedSize);

    void run() override;

    QQuickTextureFactory* textureFactory() const override;

private slots:

    void handleNetworkReply();
    void fetchFromNetwork(const QString& url);

private:

    qint64 _mRefId;
    QNetworkReply* _pReply;
    QImage _mImage;
    QSize _mRequestedSize;    
    QNetworkAccessManager* _pNetworkManager;
    ImageClass _mImageClass;

};


class MediaImageProvider : public QQuickAsyncImageProvider {
public:

    explicit MediaImageProvider() : QQuickAsyncImageProvider() {} 

    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:

    QThreadPool _mPool;

};


}
