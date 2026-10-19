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
#include "MediaImageProvider.hpp"
#include "ApplicationContext.hpp"

#include <Core/Types.hpp>
#include <Platform/Arena.hpp>
#include <Data/MediaManager.hpp>

#include <QImage>
#include <QImageReader>
#include <QColorSpace>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMetaObject>

nbl::frontend::MediaImageResponse::MediaImageResponse(qint64 refId, const QSize& requestedSize) :
    QQuickImageResponse(),
    _mRefId(refId),
    _pReply(nullptr),
    _mImage{},
    _mRequestedSize(requestedSize),
    _pNetworkManager(nullptr),
    _mImageClass(ImageClass::UNKNOWN) {

    setAutoDelete(false);
}


void nbl::frontend::MediaImageResponse::run() {

    ApplicationContext* ctx = ApplicationContext::instance();
    MediaManager* mediaManager = ctx->mediaManager();

    QByteArray* array;

    if(mediaManager->loadImage(_mRefId, [&](const nbl::ImageLoadResult& result) -> nbl::Result {

        _mImageClass = result.imageClass;

        if(result.isPresent) {
    
            QImage image(
                result.imageData.pixels.data,
                result.imageData.width,
                result.imageData.height,
                QImage::Format_RGBA8888,
                [](void* data) { delete static_cast<QByteArray*>(data); }, 
                array
            );

            if(_mRequestedSize.isValid()) {
                _mImage = image.scaled(_mRequestedSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
            } else {
                _mImage = image;
            }
            emit finished();
            return nbl::Result::SUCCESS;

        } else {

            if(result.referenceData.deviceId != 0 && result.referenceData.deviceId != ctx->deviceId()) {
                // Url cannot be loaded on this device.
                NBL_WARN("Detected an unsaved image that was local to another device!");
                return nbl::Result::NOT_FOUND;
            }

            NBL_DEBUG("Downloading ", result.referenceData.imageUrl);
            
            QMetaObject::invokeMethod(this, &MediaImageResponse::fetchFromNetwork, Qt::QueuedConnection,
                                      fromNbl(result.referenceData.imageUrl));
            return nbl::Result::SUCCESS;
        } 

    }, [&](usize req, u8** outData) -> Result {
    
        array = new QByteArray();
        array->resizeForOverwrite(req);
        *outData = reinterpret_cast<u8*>(array->data());
        return nbl::Result::SUCCESS;

    }).error()) {
        _mImage = {};
        emit finished();
    }
}


void nbl::frontend::MediaImageResponse::fetchFromNetwork(const QString& url) {

    _pNetworkManager = new QNetworkAccessManager();
    _pReply = _pNetworkManager->get(QNetworkRequest(url));
    QObject::connect(_pReply, &QNetworkReply::finished, this, &MediaImageResponse::handleNetworkReply);
}


QQuickTextureFactory* nbl::frontend::MediaImageResponse::textureFactory() const {
    if(_mImage.isNull()) {
        return nullptr;
    }
    return QQuickTextureFactory::textureFactoryForImage(_mImage);
}


void nbl::frontend::MediaImageResponse::handleNetworkReply() {

    _pNetworkManager->deleteLater();
    _pReply->deleteLater();

    if (_pReply->error() != QNetworkReply::NoError) {
        emit finished();
        return;
    }

    const QByteArray data = _pReply->readAll();

    QBuffer buffer;
    buffer.setData(data);

    if (!buffer.open(QIODevice::ReadOnly)) {
        emit finished();
        return;
    }

    QImageReader reader(&buffer);
    QImage img = reader.read();

    ImageClass::Dimensions dim = _mImageClass.dimensions();
    if(img.width() > dim.width || img.height() > dim.height) {
        img = img.scaled(
            dim.width, 
            dim.height, 
            _mImageClass.shouldCrop() ? Qt::AspectRatioMode::KeepAspectRatioByExpanding : Qt::AspectRatioMode::KeepAspectRatio, 
            Qt::SmoothTransformation);
    }

    if(img.format() != QImage::Format_RGBA8888) {
        img = img.convertToFormat(QImage::Format_RGBA8888);
    }

    QColorSpace colorSpace = img.colorSpace(); 
    if (colorSpace.isValid() && colorSpace != QColorSpace::SRgb) {
        img.convertToColorSpace(QColorSpace::SRgb);
    }

    ApplicationContext* ctx = ApplicationContext::instance();
    MediaManager* manager = ctx->mediaManager();

    const u8* imageBytes = img.constBits();
    if(manager->saveImage(_mRefId, img.width(), img.height(), {
        .data = imageBytes,
        .length = static_cast<usize>(img.sizeInBytes())
    }).error()) {
        NBL_WARN("Failed to save image!");
    }
 

    if(_mRequestedSize.isValid()) {
        img = img.scaled(_mRequestedSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    _mImage = img;
    emit finished();

}




QQuickImageResponse* nbl::frontend::MediaImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize) {

    bool ok;
    i64 refId = id.toLongLong(&ok);
    if(!ok) return nullptr;

    MediaImageResponse* out = new MediaImageResponse(refId, requestedSize);
    _mPool.start(out);
    return out;
}
