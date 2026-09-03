#pragma once

#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Platform/Process.hpp>
#include <Platform/Mutex.hpp>
#include <Plugin/MetadataProvider.hpp>
#include <Platform/Arena.hpp>



class MetadataSearchHelper : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Status status READ status NOTIFY statusUpdated);

public:

    enum Status {
        IDLE,
        SEARCHING,
        DONE
    };
    Q_ENUM(Status)


    MetadataSearchHelper(QObject* parent = nullptr) :
        QObject(parent),
        _mStatus(IDLE)
    { 
        NBL_ASSERT(nbl::Arena::create(1024 * 1024 * 4, &_mArena).success());
    }
    ~MetadataSearchHelper();

    MetadataSearchHelper(const MetadataSearchHelper& other) = delete;
    MetadataSearchHelper(MetadataSearchHelper&& other) = delete;

    MetadataSearchHelper& operator=(const MetadataSearchHelper& other) = delete;
    MetadataSearchHelper& operator=(MetadataSearchHelper&& other) = delete;


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

    friend class MetadataSearchWorker;
};


class MetadataSearchWorker : public QObject, public QRunnable {

    Q_OBJECT

public:
;

    MetadataSearchWorker(const MetadataSearchHelper* parent, const nbl::MetadataLookupInfo* lookup, const nbl::MetadataProvider* provider) :
        _pParent(parent),
        _pLookup(lookup),
        _pProvider(provider)
    { }

    void run() override;

signals:

    void done();
    void resultAvailable(quint32 type, QVariant value);

private:

    const MetadataSearchHelper* _pParent;
    const nbl::MetadataLookupInfo* _pLookup;
    const nbl::MetadataProvider* _pProvider;

};
