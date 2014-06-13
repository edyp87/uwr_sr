#include "chatclient.h"

static const quint16 defaultPort = 5432;

ChatClient::ChatClient(QWidget* parent,
                       Qt::WindowFlags flags)
    : QWidget(parent, flags), wasSearchClicked(false), startedLookingForServer(false) {
    QVBoxLayout* mainSpace      = new QVBoxLayout(this);
    QGridLayout* topSpace       = new QGridLayout;
    QHBoxLayout* bottomSpace	= new QHBoxLayout;

    widgetLabel         = new QLabel("Serwer:", this);
    widgetServer        = new QLineEdit(this);
    widgetPort			= new QSpinBox(this);
    widgetConn			= new QPushButton("Polacz", this);
    widgetSearch        = new QPushButton("Szukaj", this);
    widgetPort->setRange(1, 32767);
    widgetPort->setValue(defaultPort);

    QString interfaceAddress = lookUpForIPAddress();
    widgetServer->setText(interfaceAddress);
    topSpace->addWidget(widgetLabel , 0, 0);
    topSpace->addWidget(widgetServer, 0, 1);
    topSpace->addWidget(widgetPort  , 0, 2);
    topSpace->addWidget(widgetSearch  , 0, 3);
    widgetLabel	= new QLabel("Nick:", this);
    widgetNick	= new QLineEdit(this);
    widgetNick->setText("Anonim");
    topSpace->addWidget(widgetLabel, 1, 0);
    topSpace->addWidget(widgetNick , 1, 1);
    topSpace->addWidget(widgetConn , 1, 3);

    widgetChat	= new QTextEdit(this);
    widgetChat->setReadOnly(true);

    widgetLabel     = new QLabel("Wiadomosc:", this);
    widgetMessage	= new QLineEdit(this);
    widgetSend      = new QPushButton("Wyslij", this);
    widgetSend->setDefault(true);
    bottomSpace->addWidget(widgetLabel);
    bottomSpace->addWidget(widgetMessage);
    bottomSpace->addWidget(widgetSend);

    mainSpace->addLayout(topSpace);
    mainSpace->addWidget(widgetChat);
    mainSpace->addLayout(bottomSpace);
    setLayout(mainSpace);

    socketBuffer = new QBuffer(this);
    socketHandle = new QTcpSocket(this);
    socketHandle->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    socketBuffer->open(QIODevice::ReadWrite);
    broadcast =   new BroadcastHandler();

    connect(widgetMessage, SIGNAL(returnPressed()),                  SLOT(sendMessage()));
    connect(widgetSend,    SIGNAL(clicked()),                        SLOT(sendMessage()));
    connect(widgetConn,    SIGNAL(clicked()),                        SLOT(toggleConnection()));
    connect(widgetSearch,  SIGNAL(clicked()), broadcast,             SLOT(sendAttachRequest()));
    connect(widgetSearch,  SIGNAL(clicked()),                        SLOT(setSearchFlag()));
    connect(broadcast,     SIGNAL(serverOffer(QHostAddress)),  this, SLOT(receiveServerOffer(QHostAddress)));
    connect(broadcast,     SIGNAL(setNewServer(QHostAddress)), this, SLOT(receiveServerOffer(QHostAddress)));
    connect(&timer,        SIGNAL(timeout()),                  this, SLOT(keepAliveDoesntCameBack()));


    connect(socketHandle,  SIGNAL(connected()),                      SLOT(setConnected()));
    connect(socketHandle,  SIGNAL(disconnected()),                   SLOT(setDisconnected()));
    connect(socketHandle,  SIGNAL(readyRead()),                      SLOT(receiveMessage()));

	setDisconnected();

}

ChatClient::~ChatClient() {
    socketBuffer->close();
    // commented out because of bad order...
//    delete mainSpace;
//    delete topSpace;
//    delete bottomSpace;
//    delete widgetPort;
//    delete widgetServer;
//    delete widgetConn;
//    delete widgetSearch;
//    delete widgetLabel;
//    delete widgetNick;
//    delete widgetChat;
//    delete widgetMessage;
//    delete widgetSend;
//    delete socketBuffer;
//    delete socketHandle;
//    delete broadcast;

}

void ChatClient::setConnected() {
    widgetPort  ->setEnabled(false);
    widgetServer->setEnabled(false);
    widgetNick  ->setEnabled(true);
    widgetMessage->setEnabled(true);
    widgetChat  ->setEnabled(true);
    widgetChat  ->clear();
    widgetSend  ->setEnabled(true);
    widgetConn  ->setText("Rozlacz");
    widgetSearch->setEnabled(false);
    broadcast->setServerAddress(QHostAddress(socketHandle->peerName()));
}

void ChatClient::setDisconnected() {
    widgetPort  ->setEnabled(false);
    widgetServer->setEnabled(true);
    widgetNick  ->setEnabled(false);
    widgetMessage->setEnabled(false);
    widgetChat  ->setEnabled(false);
    widgetSend  ->setEnabled(false);
    widgetConn  ->setText("Polacz");
     widgetSearch->setEnabled(true);
     broadcast->resetServerAddress();
}

void ChatClient::toggleConnection() {

    if (socketHandle->state() == QAbstractSocket::UnconnectedState) {
        socketHandle->connectToHost(widgetServer->text(), widgetPort->value());
        wasSearchClicked = false;
        sendKeepAlive();
    }
    else {
        socketHandle->disconnectFromHost();
        timer.stop();
	}
}

void ChatClient::sendMessage() {
    socketHandle->write("<" + widgetNick->text().toLatin1() + "> " + widgetMessage->text().toLatin1() + "\n");
    widgetMessage->clear();
}

void ChatClient::sendKeepAlive() {
    socketHandle->write("KEEP\n");
    receivedKeepAlive = false;
    keepAlives = 2;
    timer.start(1000);
}

void ChatClient::keepAliveDoesntCameBack() {
    --keepAlives;

    if(keepAlives == 0) {
        if(receivedKeepAlive)
            sendKeepAlive();
        else {
            qDebug() << "Server is down!";
            toggleConnection();
            broadcast->sendServerDownInfo();
            startedLookingForServer = true;
        }
    }
}

void ChatClient::receiveMessage() {
    qint64 bytes = socketBuffer->write(socketHandle->readAll());
    socketBuffer->seek(socketBuffer->pos() - bytes);
    while (socketBuffer->canReadLine()) {
        QString line = socketBuffer->readLine();
        if(line == "ALIVE\n") {
            receivedKeepAlive = true;
        }
        else
            widgetChat->append(line.simplified());
    }
}


void ChatClient::receiveServerOffer(QHostAddress serverAddress) {
    if(wasSearchClicked || startedLookingForServer) {
        startedLookingForServer = false;
        widgetServer->setText(serverAddress.toString());
        toggleConnection();
    }
}

void ChatClient::setSearchFlag() {
    wasSearchClicked = true;
}

void ChatClient::handleServerError(QAbstractSocket::SocketError) {
    qDebug() << "--------------------- SERVER ERROR ---------------------";
}

QString ChatClient::lookUpForIPAddress() {

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QString toReturnAddress = "127.0.0.1";
    foreach(const QNetworkInterface & interface, interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp) {
            foreach(const QNetworkAddressEntry & address, interface.addressEntries()) {
                if(address.ip().toIPv4Address() != 0 && address.ip().toIPv4Address() != 2130706433)
                toReturnAddress = address.ip().toString();

            }
        }
    }
    return toReturnAddress;
}
