#include "chatclient.h"

static const quint16 defaultPort = 5432;

ChatClient::ChatClient(QSharedPointer<Peers> peersPtr,
                       QWidget* parent,
                       Qt::WindowFlags flags)
    : QWidget(parent, flags), peers(peersPtr), wasSearchClicked(false) {
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
    widgetServer->setText("localhost");
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
    socketBuffer->open(QIODevice::ReadWrite);
    broadcast = new BroadcastHandler(peers);

    connect(widgetMessage, SIGNAL(returnPressed()), SLOT(sendMessage()));
    connect(widgetSend, SIGNAL(clicked()), SLOT(sendMessage()));
    connect(widgetConn, SIGNAL(clicked()), SLOT(toggleConnection()));
    connect(widgetSearch, SIGNAL(clicked()), broadcast, SLOT(sendAttachRequest()));
    connect(widgetSearch, SIGNAL(clicked()), SLOT(setSearchFlag()));
    connect(broadcast, SIGNAL(serverOffer(QHostAddress)), this, SLOT(receiveServerOffer(QHostAddress)));

    connect(socketHandle, SIGNAL(connected()), SLOT(setConnected()));
    connect(socketHandle, SIGNAL(disconnected()), SLOT(setDisconnected()));
    connect(socketHandle, SIGNAL(readyRead()), SLOT(receiveMessage()));

	setDisconnected();
}

ChatClient::~ChatClient() {
    socketBuffer->close();
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
    widgetPort  ->setEnabled(true);
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
    }
    else {
        socketHandle->disconnectFromHost();
	}
}

void ChatClient::sendMessage() {
    socketHandle->write("<" + widgetNick->text().toLatin1() + "> " + widgetMessage->text().toLatin1() + "\n");
    widgetMessage->clear();
}

void ChatClient::receiveMessage() {
    qint64 bytes = socketBuffer->write(socketHandle->readAll());
    socketBuffer->seek(socketBuffer->pos() - bytes);
    while (socketBuffer->canReadLine()) {
        QString line = socketBuffer->readLine();
        widgetChat->append(line.simplified());
    }
}


void ChatClient::receiveServerOffer(QHostAddress serverAddress) {
    if(wasSearchClicked) {
        widgetServer->setText(serverAddress.toString());
        toggleConnection();
    }
}

void ChatClient::setSearchFlag() {
    wasSearchClicked = true;
}
