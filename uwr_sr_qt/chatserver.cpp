#include "chatserver.h"

ChatServer::ChatServer(QObject* parent)
    : QTcpServer(parent) {
	connect(this, SIGNAL(newConnection()), this, SLOT(addConnection()));
}

ChatServer::~ChatServer() {
}

void ChatServer::addConnection() {
    QTcpSocket* newConnection = nextPendingConnection();
    connectionList.append(newConnection);

	QBuffer* buffer = new QBuffer(this);
	buffer->open(QIODevice::ReadWrite);
    buffers.insert(newConnection, buffer);

    connect(newConnection, SIGNAL(disconnected()), SLOT(removeConnection()));
    connect(newConnection, SIGNAL(readyRead()),	   SLOT(receiveMessage()));

    foreach (QTcpSocket* connection, connectionList) {
        connection->write("Utworzono polaczenie... -- "
                              + newConnection->peerAddress().toString().toLatin1()
                              + "\n"
                         );
    }
}

void ChatServer::removeConnection() {
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QBuffer*    buffer = buffers.take(socket);

	buffer->close();
	buffer->deleteLater();

    connectionList.removeAll(socket);
    socket->deleteLater();
    foreach (QTcpSocket* connection, connectionList) {
        connection->write("Zakonczono polaczenie... -- "
                              + socket->peerAddress().toString().toLatin1()
                              + "\n"
                         );
    }
}

void ChatServer::receiveMessage() {
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QBuffer* buffer    = buffers.value(socket);
	
	qint64 bytes = buffer->write(socket->readAll());	
	buffer->seek(buffer->pos() - bytes);

    while (buffer->canReadLine()) {
		QByteArray line = buffer->readLine();

        if(line == "KEEP\n")
            foreach (QTcpSocket* connection, connectionList) {
                connection->write("ALIVE\n");
            }
        else
            foreach (QTcpSocket* connection, connectionList) {
                connection->write(line);
            }
	}
}

//------------------------------------------------

BroadcastHandler::BroadcastHandler(QObject * parent)
    : QUdpSocket(parent) {
    this->bind(5432, QUdpSocket::ShareAddress);
    connect(this, SIGNAL(readyRead()), this, SLOT(receivedBroadcast()));
}

void BroadcastHandler::sendResponse(QByteArray receivedMsg) {
    if(receivedMsg == "1_ImLookingForSomeFun") {
        qDebug() << "Somebody is looking company :)";

        if(serverAddress != QHostAddress("255.255.255.255")) {
            QByteArray newMessage(serverAddress.toString().toLatin1());
            this->writeDatagram(newMessage.data(), newMessage.size(), QHostAddress::Broadcast, 5432);
        }
    }
    else if(receivedMsg == "2_ServerIsDown") {
        qDebug() << "Somebody has lost his company :(";
        sendOwnCandidature();
    } else {
         if(serverAddress == QHostAddress("255.255.255.255"))
            emit serverOffer(QHostAddress(receivedMsg.data()));
    }
}

void BroadcastHandler::sendOwnCandidature() {
    QByteArray message = "MeMeMeMeeee!!!";
    this->writeDatagram(message.data(), message.size(), QHostAddress::Broadcast, 5432);
}

void BroadcastHandler::receivedBroadcast() {
    QByteArray message;
    message.resize(this->pendingDatagramSize());
    this->readDatagram(message.data(), message.size());
    qDebug() << "RECEIVED MSG: " << message.data();
    sendResponse(message);
}

void BroadcastHandler::sendAttachRequest() {
    QByteArray message = "1_ImLookingForSomeFun";
    this->writeDatagram(message.data(), message.size(), QHostAddress::Broadcast, 5432);
}

void BroadcastHandler::setServerAddress(QHostAddress serverAddr) {
    qDebug() << "SET SERVER IN BROADCAST" << serverAddr.toString().toLatin1();
    serverAddress = serverAddr;
}

void BroadcastHandler::resetServerAddress() {
    qDebug() << "RESET SERVER IN BROADCAST";
    serverAddress = QHostAddress("255.255.255.255");
}
