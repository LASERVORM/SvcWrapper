// EchoServer demo application wrapped as shared library.
// Copyright (c) LASERVORM GmbH 2023
#include "EchoServer/echoserver_main.h"
#include <QCoreApplication>
#include "echoserver.h"

static EchoServer* hSrv {nullptr};

/*
 * This is the real main function of our demo application we're going to wrap
 * as a service. It shows a typical example of a Qt console application.
 */
int echoserver_main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    EchoServer srv;
    hSrv = &srv;
    QObject::connect(&srv, &EchoServer::done,
                     &app, &QCoreApplication::exit, Qt::QueuedConnection);
    QMetaObject::invokeMethod(&srv, &EchoServer::startup, Qt::QueuedConnection);

    return app.exec();
}

/*
 * This is the shutdown handler for our demo application.
 * Note: This function must always be thread safe, that's why use a queued
 * connection call rather than invoking the slot directly.
 */
void shutdown()
{
    if (!hSrv)
        return;
    QMetaObject::invokeMethod(hSrv, &EchoServer::shutdown,
                              Qt::QueuedConnection);
}
