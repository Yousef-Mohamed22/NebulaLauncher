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


/**
 * \file Plugin/ProcessTracker.hpp
 * \author jwall64
 * \brief Keeps track of the state of individual game processes.
 */

#pragma once

#include <Core/Types.hpp>
#include <Platform/Process.hpp>
#include <Plugin/PluginContext.hpp>

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

    void pushStatus(StringView status);
    void pushStatus(StringView status, f64 progress);
    void pushProgress(f64 progress);

    Result run(PluginContext* context, StringView command, StringView workingDir);

    void check();

    void terminate();
    void kill();

    inline const ProcessHandle* handle() const { return &_mHandle; }

    static Result create(i64 installationId, void(*statusConsumer)(i64, Optional<StringView>, Optional<f64>), ProcessTracker* outTracker);

private:

    i64 _mInstallationId;
    State _mState;
    ProcessHandle _mHandle;
    void(*_fStatusConsumer)(i64, Optional<StringView>, Optional<f64>);

};


}
