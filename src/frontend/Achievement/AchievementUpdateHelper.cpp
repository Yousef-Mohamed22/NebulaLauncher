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
#include "Achievement/AchievementUpdateHelper.hpp"
#include "ApplicationContext.hpp"

#include <Plugin/AchievementProvider.hpp>

void nbl::frontend::AchievementUpdateWorker::run() {


    bool reload = false;

    ApplicationContext* ctx = ApplicationContext::instance();

    auto _handle = [&](const nbl::AchievementSet& achievementSet) {

        const nbl::AchievementProvider* provider = ctx->pluginContext()->get<nbl::AchievementProvider>(achievementSet.provider);
        if(provider == nullptr || (achievementSet.hidden && _mSetId == 0)) {
            return nbl::Result::SUCCESS;
        }

        nbl::AchievementHelper helper(ctx->databaseConnection(), provider->id, _mGameId);
        nbl::Result res = provider->updateAchievements(ctx->pluginContext(), achievementSet, helper);

        if(res.error()) {
            ctx->pluginContext()->pushNotification(nbl::NotificationType::WARNING, "An error occurred", "An error occurred while an achievement provider was updating achievements!");
            return nbl::Result::SUCCESS;
        }

        if(res != nbl::Result::NO_CHANGE) {
            reload = true;
        }
        return nbl::Result::SUCCESS;
    };

    if(_mSetId != 0) {
        if(nbl::AchievementSet::get(ctx->databaseConnection(), _mSetId, _handle).error()) {
            ctx->pluginContext()->pushNotification(nbl::NotificationType::ERROR, "An error occurred", "An error occurred while updating achievements!");
            emit done(false);
        }
    } else {
        if(nbl::AchievementSet::enumerateForGame(ctx->databaseConnection(), UINT64_MAX, 0, _mGameId, _handle).error()) {
            ctx->pluginContext()->pushNotification(nbl::NotificationType::ERROR, "An error occurred", "An error occurred while updating achievements!");
            emit done(false);
        };
    }

    emit done(reload);
}

