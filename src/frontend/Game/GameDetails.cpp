/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
