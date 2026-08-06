#include <Core/String.hpp>
#include <Plugin/ProcessTracker.hpp>
#include <Plugin/PluginContext.hpp>
#include <Platform/Assert.hpp>

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

void nbl::ProcessTracker::kill() {
    if(_mState != State::RUNNING) return;
    _mHandle.kill();
    _mState = State::KILLED;
}

nbl::Result nbl::ProcessTracker::create(i64 installationId, ProcessTracker* outTracker) {
    NBL_ASSERT(outTracker->_mState == State::NOT_STARTED);
    outTracker->_mInstallationId = installationId;
    outTracker->_mState = State::STARTING;
    return nbl::Result::SUCCESS;
}
