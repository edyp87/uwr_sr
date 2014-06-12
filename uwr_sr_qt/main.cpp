#include <QApplication>
#include "chatclient.h"
#include "chatserver.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);

    quint16 port = 5432;

    ChatServer server;
    server.listen(QHostAddress::Any, port);

    ChatClient client;
    client.show();

	return a.exec();
}
