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
#include <QTimer>
#include <set> // QT nie posiada seta z sortowaniem :(

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
    void electNewServer();

public slots:
    void sendAttachRequest();
    void sendServerDownInfo();

private slots:
    void receivedBroadcast();
    void setNewServer();

private:
    void sendResponse(QByteArray receivedMsg,  QHostAddress peerAddress);
    void sendOwnCandidature();
    QHostAddress serverAddress;
    bool isServerElecting;
    std::set<QString> * serverCandidates;
    QTimer electingServerTimer;

signals:
    void serverOffer(QHostAddress serverAddress);
    void setNewServer(QHostAddress electedServer);
};

#endif // CHATSERVER_H
