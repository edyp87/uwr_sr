#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QList>
#include <QHash>
#include <QBuffer>
#include <QTcpSocket>

class ChatServer : public QTcpServer
{
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

#endif // CHATSERVER_H
