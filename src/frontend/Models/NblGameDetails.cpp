#include <Models/NblGameDetails.hpp>
#include <ApplicationContext.hpp>

QUrl NblGameDetails::posterUrl() {
    return ApplicationContext::instance()->mediaUrl(poster(), "poster");
}
QUrl NblGameDetails::bannerUrl() {
    return ApplicationContext::instance()->mediaUrl(banner(), "banner");
}
QUrl NblGameDetails::iconUrl() {
    return ApplicationContext::instance()->mediaUrl(icon(), "icon");
}
QUrl NblGameDetails::logoUrl() {
    return ApplicationContext::instance()->mediaUrl(logo(), "logo");
}
