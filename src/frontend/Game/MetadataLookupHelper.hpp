#pragma once

#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Platform/Process.hpp>
#include <Platform/Mutex.hpp>
#include <Plugin/MetadataProvider.hpp>
#include <Platform/Arena.hpp>


namespace nbl::frontend {

class MetadataLookupHelper : public QObject {

    Q_OBJECT
    QML_NAMED_ELEMENT(MetadataLookupHelper)

    Q_PROPERTY(Status status READ status NOTIFY statusUpdated);

public:

    enum Status {
        IDLE,
        SEARCHING,
        DONE
    };
    Q_ENUM(Status)


    MetadataLookupHelper(QObject* parent = nullptr) :
        QObject(parent),
        _mArena{},
        _bCanceled(false),
        _mStatus(IDLE),
        _mThreadPool{},
        _mRemaining(0),
        _mCurrentSearch{}
    { 
        NBL_ASSERT(nbl::Arena::create(1024 * 1024 * 4, &_mArena).success());
    }
    ~MetadataLookupHelper();

    MetadataLookupHelper(const MetadataLookupHelper& other) = delete;
    MetadataLookupHelper(MetadataLookupHelper&& other) = delete;

    MetadataLookupHelper& operator=(const MetadataLookupHelper& other) = delete;
    MetadataLookupHelper& operator=(MetadataLookupHelper&& other) = delete;


    inline Status status() const noexcept { return _mStatus; }

    Q_INVOKABLE void cancelLookup();
    Q_INVOKABLE int lookup(QByteArray title, quint64 releaseDate, QVariantMap platformIds);

signals:

    void statusUpdated();
    void receivedResult(quint32 type, QVariant value);
    void done();

private slots:

    void _oneFinished();

private:
    
    nbl::Arena _mArena;
    std::atomic<bool> _bCanceled;
    Status _mStatus;
    QThreadPool _mThreadPool;
    quint32 _mRemaining;
    nbl::MetadataLookupInfo _mCurrentSearch;

    friend class MetadataLookupWorker;
};


class MetadataLookupWorker : public QObject, public QRunnable {

    Q_OBJECT

public:

    MetadataLookupWorker(const MetadataLookupHelper* parent, const nbl::MetadataLookupInfo* lookup, const nbl::MetadataProvider* provider) :
        _pParent(parent),
        _pLookup(lookup),
        _pProvider(provider)
    { }

    void run() override;

signals:

    void done();
    void resultAvailable(quint32 type, QVariant value);

private:

    const MetadataLookupHelper* _pParent;
    const nbl::MetadataLookupInfo* _pLookup;
    const nbl::MetadataProvider* _pProvider;

};

}
