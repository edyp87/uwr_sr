#include <QApplication>
#include "chatclient.h"
#include "../ChatServer/chatserver.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    quint16 port = 5432;
    ChatServer server;
    BroadcastListener listener;

    server.listen(QHostAddress::Any, port);
    listener.bind(5432, QUdpSocket::ShareAddress);

    ChatClient client;
	client.show();


	return a.exec();
}
