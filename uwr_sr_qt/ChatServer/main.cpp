#include <QCoreApplication>
#include <QHostAddress>
#include "chatserver.h"
int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	quint16 port = 5432;

    ChatServer server;
	server.listen(QHostAddress::Any, port);
    return a.exec();
}
