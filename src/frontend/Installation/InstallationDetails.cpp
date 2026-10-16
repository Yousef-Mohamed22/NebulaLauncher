#include "Installation/InstallationDetails.hpp"
#include "ApplicationContext.hpp"

QUrl nbl::frontend::InstallationDetails::_mediaUrl(qint64 refId) const {
    return ApplicationContext::instance()->mediaUrl(refId);
}
