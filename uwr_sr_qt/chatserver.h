#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QList>
#include <QHash>
#include <QBuffer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <list>
#include <QDebug>
#include <QSharedPointer>

class ChatServer : public QTcpServer {
    Q_OBJECT

public:
    ChatServer(QObject* parent = 0);
    ~ChatServer();

private slots:
	void addConnection();
	void removeConnection();
	void receiveMessage();

private:
    QList<QTcpSocket*> connectionList;
    QHash<QTcpSocket*, QBuffer*> buffers;
};


//---------------------------------------------------

class BroadcastHandler : public QUdpSocket {
    Q_OBJECT

public:
    BroadcastHandler(QObject * parent = 0);
    void setServerAddress(QHostAddress serverAddr);
    void resetServerAddress();

public slots:
    void sendAttachRequest();

private slots:
    void receivedBroadcast();

private:
    void sendResponse(QByteArray receivedMsg);
    void sendOwnCandidature();
    QHostAddress serverAddress;

signals:
    void serverOffer(QHostAddress serverAddress);
};

#endif // CHATSERVER_H
