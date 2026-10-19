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
 * \file Plugin/ProcessTracker.cpp
 * \author jwall64
 * \brief Implementation of Plugin/ProcessTracker.hpp
 */

#include <Plugin/ProcessTracker.hpp>

#include <Core/String.hpp>
#include <Core/Assert.hpp>
#include <Plugin/PluginContext.hpp>


void nbl::ProcessTracker::pushStatus(nbl::StringView status) {
    if(_mState != State::STARTING) return;
    _fStatusConsumer(_mInstallationId, status, {});
}

void nbl::ProcessTracker::pushStatus(nbl::StringView status, f64 progress) {
    if(_mState != State::STARTING) return;
    _fStatusConsumer(_mInstallationId, status, progress);
}

void nbl::ProcessTracker::pushProgress(f64 progress) {
    if(_mState != State::STARTING) return;
    _fStatusConsumer(_mInstallationId, {}, progress);
}


nbl::Result nbl::ProcessTracker::run(PluginContext* context, StringView command, StringView workingDir) {
    NBL_ASSERT(_mState == State::STARTING);

    nbl::Result res = context->runCommand(_mInstallationId, command, workingDir, &_mHandle);
    if(res.error()) {
        _mState = State::FAILED;
        return res;
    }

    _mState = State::RUNNING;
    return nbl::Result::SUCCESS;
}

void nbl::ProcessTracker::check() {
    if(_mState == State::RUNNING && !_mHandle.isRunning()) {
        _mState = State::COMPLETE;
    }
}

void nbl::ProcessTracker::terminate() {
    if(_mState != State::RUNNING) return;
    _mHandle.terminate();
}

void nbl::ProcessTracker::kill() {
    if(_mState != State::RUNNING) return;
    _mHandle.kill();
    _mState = State::KILLED;
}

nbl::Result nbl::ProcessTracker::create(i64 installationId, void(*statusConsumer)(i64, Optional<StringView>, Optional<f64>), ProcessTracker* outTracker) {
    if(outTracker->_mState != State::NOT_STARTED) return nbl::Result::INVALID_ARGUMENT;
    outTracker->_mInstallationId = installationId;
    outTracker->_mState = State::STARTING;
    outTracker->_fStatusConsumer = statusConsumer;
    return nbl::Result::SUCCESS;
}
