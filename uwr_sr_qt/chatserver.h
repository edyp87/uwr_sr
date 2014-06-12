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

class Peers;

class ChatServer : public QTcpServer {
    Q_OBJECT

public:
    ChatServer(QSharedPointer<Peers> peersPtr, QObject* parent = 0);
    ~ChatServer();

private slots:
	void addConnection();
	void removeConnection();
	void receiveMessage();

private:
    QList<QTcpSocket*> connectionList;
    QHash<QTcpSocket*, QBuffer*> buffers;
    QSharedPointer<Peers> peers;
};


//---------------------------------------------------

class BroadcastHandler : public QUdpSocket {
    Q_OBJECT

public:
    BroadcastHandler(QSharedPointer<Peers> peersPtr, QObject * parent = 0);
    void setServerAddress(QHostAddress serverAddr);
    void resetServerAddress();

public slots:
    void sendAttachRequest();

private slots:
    void receivedBroadcast();

private:
    void sendResponse(QByteArray receivedMsg);
    void sendOwnCandidature();
    QSharedPointer<Peers> peers;
    QHostAddress serverAddress;

signals:
    void serverOffer(QHostAddress serverAddress);
};


//---------------------------------------------------

class Peers {
    std::list<QHostAddress> peerList;

public:
    bool isPeer(QHostAddress peerAddress);
    void add(QHostAddress peerAddress);
    bool remove(QHostAddress peerAddress);
    bool isIpEqual(QHostAddress addrFirst, QHostAddress addrSecond);
    void debugAll();
};

#endif // CHATSERVER_H
