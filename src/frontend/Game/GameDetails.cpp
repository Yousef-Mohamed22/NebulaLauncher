#include "Game/GameDetails.hpp"
#include "ApplicationContext.hpp"

QUrl nbl::frontend::GameDetails::posterUrl() {
    return ApplicationContext::instance()->mediaUrl(poster(), "poster");
}
QUrl nbl::frontend::GameDetails::bannerUrl() {
    return ApplicationContext::instance()->mediaUrl(banner(), "banner");
}
QUrl nbl::frontend::GameDetails::iconUrl() {
    return ApplicationContext::instance()->mediaUrl(icon(), "icon");
}
QUrl nbl::frontend::GameDetails::logoUrl() {
    return ApplicationContext::instance()->mediaUrl(logo(), "logo");
}
