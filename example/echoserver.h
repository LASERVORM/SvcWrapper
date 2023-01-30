// EchoServer demo application wrapped as shared library.
// Copyright (c) LASERVORM GmbH 2023
#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QObject>
#include <QTcpServer>

class EchoServer : public QObject
{
    Q_OBJECT
public:
    EchoServer();

public:
    void startup();
    void shutdown();

signals:
    void done(int exitCode);

private slots:
    void processClient();

private:
    QTcpServer m_server;
};

#endif // ECHOSERVER_H
