#include "chatserver.h"

ChatServer::ChatServer(QObject* parent) : QTcpServer(parent)
{
	connect(this, SIGNAL(newConnection()), this, SLOT(addConnection()));
}

ChatServer::~ChatServer()
{
}

void ChatServer::addConnection()
{
    QTcpSocket* newConnection = nextPendingConnection();
    connectionList.append(newConnection);

	QBuffer* buffer = new QBuffer(this);
	buffer->open(QIODevice::ReadWrite);
    buffers.insert(newConnection, buffer);

    connect(newConnection, SIGNAL(disconnected()), SLOT(removeConnection()));
    connect(newConnection, SIGNAL(readyRead()),	   SLOT(receiveMessage()));

    peerList.add(newConnection->peerAddress());

    foreach (QTcpSocket* connection, connectionList)
    {
        connection->write("Utworzono polaczenie... -- "
                              + newConnection->peerAddress().toString().toLatin1()
                              + "\n"
                         );
    }
}

void ChatServer::removeConnection()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QBuffer*    buffer = buffers.take(socket);

	buffer->close();
	buffer->deleteLater();

    connectionList.removeAll(socket);
	socket->deleteLater();

    foreach (QTcpSocket* connection, connectionList)
    {
        connection->write("Zakonczono polaczenie... -- "
                              + socket->peerAddress().toString().toLatin1()
                              + "\n"
                         );
    }
}

void ChatServer::receiveMessage()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QBuffer* buffer    = buffers.value(socket);
	
	qint64 bytes = buffer->write(socket->readAll());	
	buffer->seek(buffer->pos() - bytes);

	while (buffer->canReadLine())
	{
		QByteArray line = buffer->readLine();
        foreach (QTcpSocket* connection, connectionList)
		{
			connection->write(line);
		}
	}
}
