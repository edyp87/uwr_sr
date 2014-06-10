#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QList>
#include <QHash>
#include <QBuffer>
#include <QTcpSocket>
#include <list>
#include <QDebug>

class Peers {
    std::list<QHostAddress> peerList;
public:
    bool isPeer(QHostAddress peerAddress) {
        for(std::list<QHostAddress>::iterator it = peerList.begin(); it != peerList.end(); ++it)
            if(*it == peerAddress)
                return true;
            else
                return false;
    }
    void add(QHostAddress peerAddress) {
         qDebug() << "Dodano " << peerAddress.toString().toLatin1() << "\n";
         peerList.push_back(peerAddress);
    }

    bool remove(QHostAddress peerAddress) {
        for(std::list<QHostAddress>::iterator it = peerList.begin(); it != peerList.end(); ++it)
            if(*it == peerAddress) {
                peerList.erase(it);
                qDebug() << "Usunieto " << peerAddress.toString().toLatin1() << "\n";
                return true;
            }
            else
                return false;
    }

};

class ChatServer : public QTcpServer
{
	Q_OBJECT

    Peers peerList;
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



#endif // CHATSERVER_H
