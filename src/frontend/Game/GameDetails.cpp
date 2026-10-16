#include "Game/GameDetails.hpp"
#include "ApplicationContext.hpp"

QUrl nbl::frontend::GameDetails::posterUrl() {
    return nbl::frontend::mediaUrl(poster());
}
QUrl nbl::frontend::GameDetails::bannerUrl() {
    return nbl::frontend::mediaUrl(banner());
}
QUrl nbl::frontend::GameDetails::iconUrl() {
    return nbl::frontend::mediaUrl(icon());
}
QUrl nbl::frontend::GameDetails::logoUrl() {
    return nbl::frontend::mediaUrl(logo());
}
