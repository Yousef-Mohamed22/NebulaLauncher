#pragma once

#include <QObject>
#include <QtQml>

#include <Platform/Process.hpp>
#include <Platform/Mutex.hpp>
#include <Platform/Arena.hpp>
#include <Plugin/GameProvider.hpp>

class OnlineGameSearchHelper : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Status status READ status NOTIFY statusUpdated)

public:
    enum Status {
        IDLE,
        SEARCHING,
        DONE
    };
    Q_ENUM(Status)


    OnlineGameSearchHelper(QObject* parent = nullptr) :
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
    ~OnlineGameSearchHelper();

    OnlineGameSearchHelper(const OnlineGameSearchHelper& other) = delete;
    OnlineGameSearchHelper(OnlineGameSearchHelper&& other) = delete;

    OnlineGameSearchHelper& operator=(const OnlineGameSearchHelper& other) = delete;
    OnlineGameSearchHelper& operator=(OnlineGameSearchHelper&& other) = delete;


    inline Status status() const noexcept { return _mStatus; }

    Q_INVOKABLE void cancelLookup();
    Q_INVOKABLE int lookup(QByteArray title);

signals:

    void statusUpdated();
    void receivedResult(QVariantMap info);
    void done();

private slots:

    void _oneFinished();

private:

    nbl::Arena _mArena;
    std::atomic<bool> _bCanceled;
    Status _mStatus;
    QThreadPool _mThreadPool;
    quint32 _mRemaining;
    nbl::OnlineGameLookupInfo _mCurrentSearch;

    friend class OnlineGameSearchWorker;
};


class OnlineGameSearchWorker : public QObject, public QRunnable {

    Q_OBJECT

public:
;

    OnlineGameSearchWorker(const OnlineGameSearchHelper* parent, const nbl::OnlineGameLookupInfo* lookup, const nbl::GameProvider* provider) :
        _pParent(parent),
        _pLookup(lookup),
        _pProvider(provider)
    { }

    void run() override;

signals:

    void done();
    void resultAvailable(QVariantMap info);

private:

    const OnlineGameSearchHelper* _pParent;
    const nbl::OnlineGameLookupInfo* _pLookup;
    const nbl::GameProvider* _pProvider;

};
