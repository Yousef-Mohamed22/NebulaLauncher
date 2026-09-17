#include "AchievementUpdateHelper.hpp"
#include "ApplicationContext.hpp"
#include "Plugin/AchievementProvider.hpp"

void AchievementUpdateWorker::run() {


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
        if(nbl::getAchievementSet(ctx->databaseConnection(), _mSetId, _handle).error()) {
            ctx->pluginContext()->pushNotification(nbl::NotificationType::ERROR, "An error occurred", "An error occurred while updating achievements!");
            emit done(false);
        }
    } else {
        if(nbl::enumerateGameAchievementSets(ctx->databaseConnection(), UINT64_MAX, 0, _mGameId, _handle).error()) {
            ctx->pluginContext()->pushNotification(nbl::NotificationType::ERROR, "An error occurred", "An error occurred while updating achievements!");
            emit done(false);
        };
    }

    emit done(reload);
}

