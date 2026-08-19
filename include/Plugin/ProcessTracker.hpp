#pragma once

#include <Core/Types.hpp>
#include <Platform/Process.hpp>
#include <Plugin/PluginContext.hpp>

#include <functional>

namespace nbl {


class ProcessTracker {
public:
    enum class State {
        NOT_STARTED = 0,
        STARTING,
        RUNNING,
        FAILED,
        COMPLETE,
        KILLED
    };

    inline State state() const { return _mState; }

    inline void markFailed() {
        NBL_ASSERT(_mState == State::STARTING);
        _mState = State::FAILED;
    }

    inline void markComplete() {
        NBL_ASSERT(_mState == State::STARTING);
        _mState = State::COMPLETE;
    }

    void pushStatus(StringView status, f64 progress);

    Result run(PluginContext* context, StringView command, StringView workingDir);

    void check();

    void kill();

    inline const ProcessHandle* handle() const { return &_mHandle; }

    static Result create(i64 installationId, void(*statusConsumer)(i64, StringView, f64), ProcessTracker* outTracker);

private:

    i64 _mInstallationId;
    State _mState;
    ProcessHandle _mHandle;
    void(*_fStatusConsumer)(i64, StringView, f64);

};


}
