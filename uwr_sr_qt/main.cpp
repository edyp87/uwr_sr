#include <QApplication>
#include "chatclient.h"
#include "chatserver.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);

    quint16 port = 5432;

    QSharedPointer<Peers> peersPtr(new Peers);

    ChatServer server(peersPtr);
    server.listen(QHostAddress::Any, port);

    ChatClient client(peersPtr);
    client.show();

	return a.exec();
}
