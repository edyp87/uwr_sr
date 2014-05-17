#include <QCoreApplication>
#include <QHostAddress>
#include "chatserver.h"

static const quint16 DEFAULT_PORT = 6667;

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	quint16 port = 0;
	bool ok = false;
	if (argc > 0)
	{
		port = QString(argv[0]).toInt(&ok);
	}
	if (!ok)
	{
		port = DEFAULT_PORT;
	}
    ChatServer server;
	server.listen(QHostAddress::Any, port);
	return a.exec();
}
