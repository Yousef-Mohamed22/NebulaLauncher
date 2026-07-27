#pragma once

#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Platform/Process.hpp>

class NblProcessHandle : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool hasStarted READ hasStarted NOTIFY hasStartedUpdated)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningUpdated)
    Q_PROPERTY(bool isStopping READ isStopping NOTIFY isStoppingUpdated)
    Q_PROPERTY(QString logs READ logs NOTIFY logsUpdated)

public:

    NblProcessHandle(QObject* parent = nullptr) :
        QObject(parent),
        _bHasStarted(false),
        _bWasRunning(false),
        _bIsStopping(false),
        _mHandle{},
        _mWatcherThread{},
        _mLogs("")
    { }

    ~NblProcessHandle() {
        if(_mWatcherThread.isRunning()) {
            _mWatcherThread.join(nullptr);
        }
    }

    inline bool hasStarted() {
        return _bHasStarted;
    }

    inline bool isRunning() {
        return _bWasRunning;
    }

    inline bool isStopping() {
        return _bIsStopping;
    }

    Q_INVOKABLE void stop() {

        if(_bIsStopping) return;

        _bIsStopping = true;
        isStoppingUpdated();

        _mHandle.kill();
    }

    Q_INVOKABLE void check() {

        bool nowRunning = _mHandle.isRunning();
        if(nowRunning != _bWasRunning) {

            if(nowRunning && !_bHasStarted) {
                _bHasStarted = true;
                hasStartedUpdated();
            }
            _bWasRunning = nowRunning;
            isRunningUpdated();

            if(!nowRunning) {
                emit stopped();
            }
        }
    }

    inline nbl::ProcessHandle* handle() { return &_mHandle; } 

    inline QString logs() {
        return _mLogs;
    }

    inline void appendLine(QString logLine) {
        _mLogs += logLine + '\n';
        logsUpdated();
    }

    nbl::Result initLogging();

signals:

    void hasStartedUpdated();
    void isRunningUpdated();
    void isStoppingUpdated();
    void logsUpdated();
    void stopped();

private:

    bool _bHasStarted;
    bool _bWasRunning;
    bool _bIsStopping;
    nbl::ProcessHandle _mHandle;
    nbl::Thread _mWatcherThread;
    QString _mLogs;

};
