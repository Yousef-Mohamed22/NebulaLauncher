#include "MetadataSearchHelper.hpp"
#include "ApplicationContext.hpp"

#include <Plugin/MetadataProvider.hpp>


void MetadataSearchWorker::run() {

    nbl::Result res = _pProvider->provide(ApplicationContext::instance()->pluginContext(), *_pLookup, [&](nbl::MetadataField field, nbl::MetadataValue value) -> nbl::Result {

        if(_pParent->_bCanceled) return nbl::Result::CANCELED;

        quint32 type = static_cast<quint32>(field);
        QVariant val;
        switch(field) {
            case nbl::MetadataField::TITLE:
            case nbl::MetadataField::DESCRIPTION:
            case nbl::MetadataField::POSTER:
            case nbl::MetadataField::BANNER:
            case nbl::MetadataField::ICON:
            case nbl::MetadataField::LOGO:
            case nbl::MetadataField::TAG:
                val = QString(fromNbl(value.string));
                break;
            case nbl::MetadataField::RELEASE_DATE:
                val = static_cast<quint64>(value.releaseDate);
                break;
            case nbl::MetadataField::RATING:
                val = value.rating;
                break;
        }
        {
            emit resultAvailable(type, val);
        }
        return nbl::Result::SUCCESS;
    });
    if(res.error()) {
        ApplicationContext::instance()->pluginContext()->pushNotification(nbl::NotificationType::WARNING, "Lookup Failed", "A metadata provider encountered an error while searching for metadata");
        nbl::log(nbl::LogLevel::ERROR, "MetadataProvider \"", _pProvider->name, "\" encountered an error while searching for metadata: ", res);
    }

    emit done();
}



MetadataSearchHelper::~MetadataSearchHelper() {
    cancelLookup();
}


void MetadataSearchHelper::cancelLookup() {

    if(_mStatus == SEARCHING) {
        _bCanceled = true;
        _mThreadPool.clear();
        _mThreadPool.waitForDone();
        _mArena.popTo(0);

        _mStatus = IDLE;
        emit statusUpdated();
    }
}

void MetadataSearchHelper::_oneFinished() {
    _mRemaining--;
    if(_mRemaining == 0) {
        _mStatus = DONE;
        emit statusUpdated();
        emit done();
    }
}

int MetadataSearchHelper::lookup(QByteArray title, quint64 releaseDate, QVariantMap platformIds) {

    cancelLookup();
   
    _mStatus = SEARCHING;
    emit statusUpdated();

    ApplicationContext* ctx = ApplicationContext::instance();

    if(ctx->pluginContext()->registry<nbl::MetadataProvider>().length == 0) {
        emit done();
        return nbl::Result::SUCCESS;
    } 

    nbl::MetadataLookupInfo& info = _mCurrentSearch;

    if(title.length() > 0) {
        void* data;
        _mArena.push(title.length(), &data);
        memcpy(data, title.data(), title.length());
        info.title = nbl::StringView(static_cast<char*>(data), title.length());
    }
    if(releaseDate > 0) {
        info.releaseDate = releaseDate;
    }

    if(platformIds.count() > 0) {
        nbl::StringView* names;
        nbl::StringView* ids;
        NBL_CHECK(_mArena.allocate(&names, platformIds.count()));
        NBL_CHECK(_mArena.allocate(&ids, platformIds.count()));

        usize index = 0;
        for(QString key : platformIds.keys()) {
            QByteArray arr = key.toUtf8();
            void* data;
            NBL_CHECK(_mArena.push(arr.length(), &data));

            memcpy(data, arr.data(), arr.length());
            names[index] = nbl::StringView(static_cast<char*>(data), arr.length());

            QByteArray value = platformIds[key].toString().toUtf8();
            NBL_CHECK(_mArena.push(value.length(), &data));

            memcpy(data, value.data(), value.length());
            ids[index] = nbl::StringView(static_cast<char*>(data), value.length());

            index++;
        }

        info.platformIds = nbl::Span<nbl::StringView>(ids, platformIds.count());
        info.platformNames = nbl::Span<nbl::StringView>(names, platformIds.count());
    }


    nbl::PluginContext* context = ApplicationContext::instance()->pluginContext();
    for(const nbl::MetadataProvider* provider : context->registry<nbl::MetadataProvider>()) {
        if(provider->canProvide(context, info)) {

            MetadataSearchWorker* worker = new MetadataSearchWorker(this, &info, provider);

            QObject::connect(worker, &MetadataSearchWorker::resultAvailable, this, &MetadataSearchHelper::receivedResult);
            QObject::connect(worker, &MetadataSearchWorker::done, this, &MetadataSearchHelper::_oneFinished);

            _mRemaining++;
            _mThreadPool.start(worker);
        }
    }

    return nbl::Result::SUCCESS;
}
