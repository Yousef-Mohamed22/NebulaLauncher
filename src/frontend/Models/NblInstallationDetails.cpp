#include <Models/NblInstallationDetails.hpp>
#include <ApplicationContext.hpp>

QUrl NblInstallationDetails::_mediaUrl(QByteArray rel, QByteArray repo) const {
    return ApplicationContext::instance()->mediaUrl(rel, repo);
}
