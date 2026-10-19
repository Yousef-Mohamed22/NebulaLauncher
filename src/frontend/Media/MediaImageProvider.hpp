/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
