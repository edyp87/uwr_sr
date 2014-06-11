#include "chatserver.h"

ChatServer::ChatServer(QSharedPointer<Peers> peersPtr, QObject* parent)
    : QTcpServer(parent), peers(peersPtr) {
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

    peers->add(newConnection->peerAddress());

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
    peers->remove(socket->peerAddress());
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
        foreach (QTcpSocket* connection, connectionList) {
			connection->write(line);
		}
	}
}

//------------------------------------------------

BroadcastHandler::BroadcastHandler(QSharedPointer<Peers> peersPtr,
                                   QObject * parent)
    : QUdpSocket(parent), peers(peersPtr) {
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

        peers->debugAll();
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


//------------------------------------------------

bool Peers::isPeer(QHostAddress peerAddress) {
    foreach(const QHostAddress & peer, peerList)
        if(isIpEqual(peer, peerAddress))
            return true;
    return false;
}
void Peers::add(QHostAddress peerAddress) {
     qDebug() << "Dodano " << peerAddress.toString().toLatin1() << "\n";
     peerList.push_back(peerAddress);
}

bool Peers::remove(QHostAddress peerAddress) {
    for(std::list<QHostAddress>::iterator it = peerList.begin(); it != peerList.end(); ++it)
        if(*it == peerAddress) {
            peerList.erase(it);
            qDebug() << "Usunieto " << peerAddress.toString().toLatin1() << "\n";
            return true;
        }
    return false;
}

bool Peers::isIpEqual(QHostAddress addrFirst, QHostAddress addrSecond) {
    return addrFirst == addrSecond ? true : false;
}

void Peers::debugAll() {
    qDebug() << "\nElementy kontenera Peers:";
    foreach(const QHostAddress & peer, peerList)
        qDebug() << " -- " << peer.toString().toLatin1();
}

