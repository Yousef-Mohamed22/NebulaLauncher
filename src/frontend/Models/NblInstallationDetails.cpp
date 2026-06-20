#include <Models/NblInstallationDetails.hpp>
#include <ApplicationContext.hpp>

QByteArray NblInstallationDetails::posterPath() {
    QByteArray rel = poster();
    if(rel.length() == 0) return {};
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/poster/" + rel).toUtf8();
}
QByteArray NblInstallationDetails::bannerPath() {
    QByteArray rel = banner();
    if(rel.length() == 0) return {};
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/banner/" + rel).toUtf8();
}
QByteArray NblInstallationDetails::iconPath() {
    QByteArray rel = icon();
    if(rel.length() == 0) return {};
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/icon/" + rel).toUtf8();
}
QByteArray NblInstallationDetails::logoPath() {
    QByteArray rel = logo();
    if(rel.length() == 0) return {};
    return ("file://" + ApplicationContext::instance()->dataFolder() + "/media/logo/" + rel).toUtf8();
}
