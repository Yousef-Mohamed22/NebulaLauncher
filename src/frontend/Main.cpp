// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Main.cpp
 * \author jwall64
 * \brief Main application entry point.
 */

#include <qqml.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <Core/String.hpp>
#include <Platform/Logging.hpp>

#include "Media/MediaImageProvider.hpp"
#include "ApplicationContext.hpp"

int run(int argc, char** argv) {
    
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine{};

    engine.addImageProvider(QLatin1String("nblmedia"), new nbl::frontend::MediaImageProvider());

    ApplicationContext* inst = ApplicationContext::instance();
    QQmlEngine::setObjectOwnership(inst, QQmlEngine::CppOwnership);
    inst->init();

    engine.loadFromModule("Nebula", "MainWindow");

    return app.exec();
}

int main(int argc, char** argv) {

    int rc = run(argc, argv);
    if(rc != 0) {
        nbl::log(nbl::LogLevel::ERROR, "Application error: ", rc);
        return rc;
    }

    return 0;
}
