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

#include <QObject>
#include <QtQml>

#include <Platform/Process.hpp>
#include <Platform/Mutex.hpp>
#include <Platform/Arena.hpp>
#include <Plugin/GameProvider.hpp>


namespace nbl::frontend {

class OnlineGameLookupHelper : public QObject {

    Q_OBJECT
    QML_NAMED_ELEMENT(OnlineGameLookupHelper)

    Q_PROPERTY(Status status READ status NOTIFY statusUpdated)

public:
    enum Status {
        IDLE,
        SEARCHING,
        DONE
    };
    Q_ENUM(Status)


    OnlineGameLookupHelper(QObject* parent = nullptr) :
        QObject(parent),
        _mArena{},
        _bCanceled(false),
        _mStatus(IDLE),
        _mThreadPool{},
        _mRemaining(0),
        _mCurrentLookup{}
    { 
        NBL_ASSERT(nbl::Arena::create(1024 * 1024 * 4, &_mArena).success());
    }
    ~OnlineGameLookupHelper();

    OnlineGameLookupHelper(const OnlineGameLookupHelper& other) = delete;
    OnlineGameLookupHelper(OnlineGameLookupHelper&& other) = delete;

    OnlineGameLookupHelper& operator=(const OnlineGameLookupHelper& other) = delete;
    OnlineGameLookupHelper& operator=(OnlineGameLookupHelper&& other) = delete;


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
    nbl::OnlineGameLookupInfo _mCurrentLookup;

    friend class OnlineGameLookupWorker;
};


class OnlineGameLookupWorker : public QObject, public QRunnable {

    Q_OBJECT

public:
;

    OnlineGameLookupWorker(const OnlineGameLookupHelper* parent, const nbl::OnlineGameLookupInfo* lookup, const nbl::GameProvider* provider) :
        _pParent(parent),
        _pLookup(lookup),
        _pProvider(provider)
    { }

    void run() override;

signals:

    void done();
    void resultAvailable(QVariantMap info);

private:

    const OnlineGameLookupHelper* _pParent;
    const nbl::OnlineGameLookupInfo* _pLookup;
    const nbl::GameProvider* _pProvider;

};

}
