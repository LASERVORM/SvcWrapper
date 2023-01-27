#include "echoserver.h"
#include <QDebug>
#include <QTcpSocket>

EchoServer::EchoServer()
    : QObject{nullptr},
      m_server(this)
{
    m_server.setMaxPendingConnections(1);
    connect(&m_server, &QTcpServer::newConnection, this, &EchoServer::processClient);
}

void EchoServer::startup()
{
    bool res = m_server.listen(QHostAddress::LocalHost, 12345);
    if (res) {
        qInfo() << "Server listening on port" << m_server.serverPort();
        return;
    }

    qCritical() << "Server startup failed!";
    emit done(1);
}

void EchoServer::shutdown()
{
    m_server.close();
    emit done(0);
}

void EchoServer::processClient()
{
    QTcpSocket* c = m_server.nextPendingConnection();
    if (!c)
        return;
    qInfo().noquote().nospace()
            << "Accepted new client: "
            << c->peerAddress().toString() << ":" << c->peerPort();

    connect(c, &QTcpSocket::readyRead, this, [c](){
        QByteArray data = c->readLine();
        qInfo() << "Replying to" << data;
        data.prepend("reply ");
        c->write(data);
        c->flush();
    });
    connect(c, &QTcpSocket::disconnected, this, [c](){
        qInfo().noquote().nospace()
                << c->peerAddress().toString() << ":" << c->peerPort()
                << " disconnected!";
        c->close();
        c->deleteLater();
    });
}
