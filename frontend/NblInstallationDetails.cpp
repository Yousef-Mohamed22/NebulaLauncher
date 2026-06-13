#include "NblInstallationDetails.hpp"
#include "ApplicationContext.hpp"

QString NblInstallationDetails::posterPath() {
    return "file://" + ApplicationContext::instance()->dataFolder() + "/media/poster/" + poster();
}
QString NblInstallationDetails::bannerPath() {
    return "file://" + ApplicationContext::instance()->dataFolder() + "/media/banner/" + banner();
}
QString NblInstallationDetails::iconPath() {
    return "file://" + ApplicationContext::instance()->dataFolder() + "/media/icon/" + icon();
}
QString NblInstallationDetails::logoPath() {
    return "file://" + ApplicationContext::instance()->dataFolder() + "/media/logo/" + logo();
}
