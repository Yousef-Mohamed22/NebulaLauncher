// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Main.cpp
 * \author jwall64
 * \brief Main application entry point.
 */

#include <Core/String.hpp>

#include <Platform/Logging.hpp>
#include <Platform/File.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Socket.hpp>
#include <Platform/StringBuilder.hpp>
#include <Platform/Process.hpp>

#include <Codec/Json.hpp>

#include <Data/Database.hpp>
#include <Data/Model.hpp>

#include <curl/curl.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int run(int argc, char** argv) {
    
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

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
