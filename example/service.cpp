// SvcWrapper demo service example.
// This version of the example demonstrates wrapping our little application
// into a fully functional Windows service using SvcWrapper.
// Copyright (c) LASERVORM GmbH 2023
#include <SvcWrapper/svcwrapper.h>
#include <EchoServer/echoserver_main.h>
#include <QDebug>

// Example log handler
void logHandler(SvcLogLevel level, const char* msg)
{
    switch (level) {
    case Critical:
        qCritical() << "Critical:" << msg;
        break;
    case Warning:
        qWarning() << "Warning:" << msg;
        break;
    case Info:
        qInfo() << "Info:" << msg;
        break;
    case Debug:
        qDebug() << "Debug:" << msg;
        break;
    }
}

int main(int argc, char* argv[])
{   
    // Create a configuration for our service
    SvcWrapperConfig cfg;
    cfg.svcName = "EchoServer";
    cfg.svcDisplayName = "EchoServer example";
    cfg.svcDescription = "Just a demonstration of the SvcWrapper library!";
    cfg.svcCallbackMain = echoserver_main;
    cfg.svcCallbackStop = shutdown;
    cfg.svcLogCallback = logHandler;

    // Execute the service wrapper
    return SvcWrapper(argc, argv, cfg);
}
