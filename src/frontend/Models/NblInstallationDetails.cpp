#include <Models/NblInstallationDetails.hpp>
#include <ApplicationContext.hpp>

QUrl NblInstallationDetails::posterUrl() {
    return ApplicationContext::instance()->mediaUrl(poster(), "poster");
}
QUrl NblInstallationDetails::bannerUrl() {
    return ApplicationContext::instance()->mediaUrl(banner(), "banner");
}
QUrl NblInstallationDetails::iconUrl() {
    return ApplicationContext::instance()->mediaUrl(icon(), "icon");
}
QUrl NblInstallationDetails::logoUrl() {
    return ApplicationContext::instance()->mediaUrl(logo(), "logo");
}
