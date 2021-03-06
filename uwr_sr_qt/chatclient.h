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
#include <QTimer>
#include <QNetworkInterface>
#include "chatserver.h"

class ChatClient : public QWidget {
	Q_OBJECT

public:
    ChatClient(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~ChatClient();

private slots:
	void setConnected();
	void setDisconnected();
	void toggleConnection();
	void sendMessage();
	void receiveMessage();
    void receiveServerOffer(QHostAddress serverAddress);
    void setSearchFlag();
    void handleServerError(QAbstractSocket::SocketError);
    void sendKeepAlive();
    void keepAliveDoesntCameBack();
private:

    QString lookUpForIPAddress();
	

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
    QTimer          timer;
    qint8           keepAlives;
    bool            receivedKeepAlive;
    bool            wasSearchClicked;
    bool startedLookingForServer;
};

#endif // ChatClient_H
