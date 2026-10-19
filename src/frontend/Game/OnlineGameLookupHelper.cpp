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
#include "OnlineGameLookupHelper.hpp"
#include "ApplicationContext.hpp"

#include <Core/Math.hpp>
#include <Core/String.hpp>
#include <Plugin/GameProvider.hpp>



namespace {

enum _ParamClassification {
    CLASS_ID,
    CLASS_TITLE,
    CLASS_INVALID
};


_ParamClassification _classify(nbl::StringView id, usize* sep) {
    if(id.length == 0) return CLASS_INVALID;

    bool foundSep = false;
    usize index = 0;
    for(char c : id) {
        if(c == ':') {
            if(index == 0) return CLASS_TITLE;
            *sep = index;
            return CLASS_ID;
        }
        else if((c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '_') {
            return CLASS_TITLE;
        }
        index++;
    }
    return CLASS_TITLE;
}


};



void nbl::frontend::OnlineGameLookupWorker::run() {

    nbl::Result res = _pProvider->provide(ApplicationContext::instance()->pluginContext(), *_pLookup, [&](const nbl::OnlineGameMeta& value) -> nbl::Result {
        if(_pParent->_bCanceled) return nbl::Result::CANCELED;

        QVariantMap out = {};
        out["title"] = QString(fromNbl(value.title));
        out["description"] = QString(fromNbl(value.description));
        out["posterUrl"] = QString(fromNbl(value.posterUrl));
        out["iconUrl"] = QString(fromNbl(value.iconUrl));
        out["bannerUrl"] = QString(fromNbl(value.bannerUrl));
        out["logoUrl"] = QString(fromNbl(value.logoUrl));
        out["logoPosition"] = static_cast<quint32>(value.logoPosition);
        out["releaseDate"] = static_cast<quint64>(value.releaseDate);
        out["rating"] = value.rating;
        
        QStringList tags = {};
        for(nbl::StringView tag : value.tags) {
            tags.append(QString(fromNbl(tag)));
        }
        out["tags"] = tags;
        
        QVariantList platformIds = {};
        for(usize i = 0 ; i < nbl::min(value.platformIds.length, value.platformNames.length) ; i++) {
            QVariantMap id = {};
            id["platform"] = QString::fromUtf8(value.platformNames[i].data, value.platformNames[i].length);
            id["id"] = QString::fromUtf8(value.platformIds[i].data, value.platformIds[i].length);
            platformIds.append(id);
        }
        out["externalIds"] = platformIds;

        emit resultAvailable(out);
        return nbl::Result::SUCCESS;
    });
    if(res.error()) {
        ApplicationContext::instance()->pluginContext()->pushNotification(nbl::NotificationType::WARNING, "Lookup Failed", "A game provider encountered an error while searching for metadata");
        nbl::log(nbl::LogLevel::ERROR, "GameProvider \"", _pProvider->name, "\" encountered an error while searching for metadata: ", res);
    }

    emit done();
}


nbl::frontend::OnlineGameLookupHelper::~OnlineGameLookupHelper() {
    cancelLookup();
}


void nbl::frontend::OnlineGameLookupHelper::cancelLookup() {

    if(_mStatus == SEARCHING) {
        _bCanceled = true;
        _mThreadPool.clear();
        _mThreadPool.waitForDone();
        _mArena.popTo(0);

        _mStatus = IDLE;
        _bCanceled = false;
        emit statusUpdated();
    }
}

void nbl::frontend::OnlineGameLookupHelper::_oneFinished() {
    _mRemaining--;
    if(_mRemaining == 0) {
        _mStatus = DONE;
        emit statusUpdated();
        emit done();
    }
}

int nbl::frontend::OnlineGameLookupHelper::lookup(QByteArray title) {

    cancelLookup();
   
    _mStatus = SEARCHING;
    emit statusUpdated();

    ApplicationContext* ctx = ApplicationContext::instance();

    if(ctx->pluginContext()->registry<nbl::GameProvider>().length == 0) {
        emit done();
        return nbl::Result::SUCCESS;
    } 

    nbl::OnlineGameLookupInfo& info = _mCurrentLookup;

    if(title.length() == 0) return nbl::Result::INVALID_ARGUMENT;

    nbl::StringView titleView(title.data(), title.length());

    usize sep = 0;
    _ParamClassification c = _classify(titleView, &sep);

    void* data;
    _mArena.push(title.length() - sep, &data);
    memcpy(data, title.data() + sep, title.length() - sep);
    info.param = nbl::StringView(static_cast<char*>(data), title.length() - sep);
    info.meaning = c == CLASS_TITLE ? nbl::OnlineGameLookupInfo::TITLE : nbl::OnlineGameLookupInfo::ID;


    nbl::PluginContext* context = ApplicationContext::instance()->pluginContext();
    for(const nbl::GameProvider* provider : context->registry<nbl::GameProvider>()) {
        if((c == CLASS_TITLE || provider->id == titleView.slice(0, sep)) && provider->canProvide(context, info)) {

            OnlineGameLookupWorker* worker = new OnlineGameLookupWorker(this, &info, provider);

            QObject::connect(worker, &OnlineGameLookupWorker::resultAvailable, this, &OnlineGameLookupHelper::receivedResult);
            QObject::connect(worker, &OnlineGameLookupWorker::done, this, &OnlineGameLookupHelper::_oneFinished);

            _mRemaining++;
            _mThreadPool.start(worker);
        }
    }

    return nbl::Result::SUCCESS;
}
