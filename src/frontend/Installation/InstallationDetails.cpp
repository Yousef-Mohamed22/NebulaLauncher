#include "Installation/InstallationDetails.hpp"
#include "ApplicationContext.hpp"

QUrl nbl::frontend::InstallationDetails::_mediaUrl(QByteArray rel, QByteArray repo) const {
    return ApplicationContext::instance()->mediaUrl(rel, repo);
}
