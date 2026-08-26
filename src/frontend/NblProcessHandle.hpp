#pragma once

#include <qtmetamacros.h>
#include <QObject>
#include <QtQml>

#include <Platform/Process.hpp>
#include <Plugin/ProcessTracker.hpp>

class NblProcessHandle : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningUpdated)
    Q_PROPERTY(bool isStopping READ isStopping NOTIFY isStoppingUpdated)
    Q_PROPERTY(QString logs READ logs NOTIFY logsUpdated)

public:

    NblProcessHandle(QObject* parent = nullptr) :
        QObject(parent),
        _bIsStopping(false),
        _mTracker{},
        _mWatcherThread{},
        _mLogs("")
    { }

    ~NblProcessHandle() {
        if(_mWatcherThread.isRunning()) {
            _mWatcherThread.join(nullptr);
        }
    }

    inline bool isRunning() {
        return _mPrevState == nbl::ProcessTracker::State::RUNNING;
    }

    inline bool isStopping() {
        return _bIsStopping;
    }

    Q_INVOKABLE void stop() {

        if(_bIsStopping) return;

        _bIsStopping = true;
        emit isStoppingUpdated();

        _mTracker.terminate();
    }

    Q_INVOKABLE void check() {

        _mTracker.check();
        nbl::ProcessTracker::State state = _mTracker.state();

        if(state != _mPrevState) {

            if(_mPrevState == nbl::ProcessTracker::State::RUNNING) {
                _mPrevState = state;
                emit isRunningUpdated();
            } else {
                _mPrevState = state;
                if(state == nbl::ProcessTracker::State::RUNNING) {
                    emit isRunningUpdated();
                }
            }
            if(state == nbl::ProcessTracker::State::COMPLETE) {
                emit complete();
                emit stopped();
            }
            else if(state == nbl::ProcessTracker::State::FAILED) {
                emit failed();
                emit stopped();
            }
        }
    }

    inline nbl::ProcessTracker* tracker() { return &_mTracker; }

    inline QString logs() {
        return _mLogs;
    }

    inline void appendLine(QString logLine) {
        _mLogs += logLine + '\n';
        emit logsUpdated();
    }

    nbl::Result initLogging();

    inline void pushStatus(QString message) {
        emit statusUpdated(message);
    }
    inline void pushProgress(f64 progress) {
        emit progressUpdated(progress);
    }

signals:

    void isRunningUpdated();
    void isStoppingUpdated();
    void logsUpdated();
    void stopped();
    void complete();
    void failed();
    void statusUpdated(QString message);
    void progressUpdated(double progress);

private:

    bool _bIsStopping;
    nbl::ProcessTracker _mTracker;
    nbl::ProcessTracker::State _mPrevState;
    nbl::Thread _mWatcherThread;
    QString _mLogs;

};
