#include <Models/NblInstallationDetails.hpp>
#include <ApplicationContext.hpp>

QByteArray NblInstallationDetails::posterPath() {
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/poster/" + poster()).toUtf8();
}
QByteArray NblInstallationDetails::bannerPath() {
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/banner/" + banner()).toUtf8();
}
QByteArray NblInstallationDetails::iconPath() {
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/icon/" + icon()).toUtf8();
}
QByteArray NblInstallationDetails::logoPath() {
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/logo/" + logo()).toUtf8();
}
