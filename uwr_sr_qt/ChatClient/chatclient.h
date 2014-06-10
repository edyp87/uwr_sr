#ifndef ChatClient_H
#define ChatClient_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QTcpSocket>
#include <QBuffer>
#include <QSpinBox>
#include <QErrorMessage>
#include <QHostAddress>
#include "../ChatServer/chatserver.h"

class ChatClient : public QWidget {
	Q_OBJECT

public:
    ChatClient(QSharedPointer<Peers> peersPtr, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~ChatClient();

private slots:
	void setConnected();
	void setDisconnected();
	void toggleConnection();
	void sendMessage();
	void receiveMessage();

private:
	

    QBuffer*		socketBuffer;
    QTcpSocket*		socketHandle;
    QSpinBox*		widgetPort;
    QLineEdit*		widgetNick;
    QLineEdit*		widgetServer;
    QLineEdit*		widgetMessage;
    QTextEdit*		widgetChat;
    QPushButton*	widgetSearch;
    QPushButton*	widgetConn;
    QPushButton*	widgetSend;
    QLabel*         widgetLabel;
    QVBoxLayout*    mainSpace;
    QGridLayout*    topSpace;
    QHBoxLayout*    bottomSpace;

    BroadcastHandler* broadcast;
    QSharedPointer<Peers> peers;
};

#endif // ChatClient_H
