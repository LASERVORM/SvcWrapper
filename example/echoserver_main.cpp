#include "EchoServer/echoserver_main.h"
#include <QCoreApplication>
#include "echoserver.h"

static EchoServer* hSrv {nullptr};

int echoserver_main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    EchoServer srv;
    hSrv = &srv;
    QObject::connect(&srv, &EchoServer::done, &app, &QCoreApplication::exit, Qt::QueuedConnection);
    QMetaObject::invokeMethod(&srv, &EchoServer::startup, Qt::QueuedConnection);

    return app.exec();
}

void shutdown()
{
    if (!hSrv)
        return;
    QMetaObject::invokeMethod(hSrv, &EchoServer::shutdown, Qt::QueuedConnection);
}
