#include "rpsgameclient.h"
#include "connection.h"

rpsGameClient::rpsGameClient(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket();
    QObject::connect(socket, &QAbstractSocket::errorOccurred, this, &rpsGameClient::tcpSocketError);
    QObject::connect(socket, &QAbstractSocket::connected, this, &rpsGameClient::sendName);
    QObject::connect(socket, &QAbstractSocket::readyRead, this, &rpsGameClient::getMessage);
}

void rpsGameClient::connect() { //socket will emit whether or not it was successful.. main() is connected to it
    socket->connectToHost(Connection::ipAddress, Connection::port);
}

void rpsGameClient::disconnect() {
    //Only called when receiving a quit() signal from the window
    socket->write("Q||");
    socket->abort();
}

void rpsGameClient::tcpSocketError(QAbstractSocket::SocketError error) { //Socket failed initial login
    std::fprintf(stderr, "Server connection error.");
    emit connectionError(error);
}

bool rpsGameClient::isMessageValid(std::string message) {
    if (message[1] != '|') return false;
    std::string::size_type msgLength = message.length();
    switch (message[0]) {
    case 'B':
        return msgLength >= 3 && msgLength <= 66;
    case 'R':
        return msgLength >= 6 && msgLength <= 12;
    case 'W':
        return msgLength == 3;
    }
    return false;
}

void rpsGameClient::sendName() {
    QByteArray msg = QByteArray();
    msg.append("P|");
    msg.append(Connection::username.toStdString().substr(0, 64));
    msg.append("||");
    socket->write(msg);
}

void rpsGameClient::sendAttack(char attack) {
    QByteArray msg = QByteArray();
    msg.append("M|");
    switch (attack) {
    case 'R':
        msg.append("ROCK");
        break;
    case 'P':
        msg.append("PAPER");
        break;
    case 'S':
        msg.append("SCISSORS");
    }
    msg.append("||");
    socket->write(msg);
}

void rpsGameClient::sendRematch() {
    socket->write("C||");
}

void rpsGameClient::getMessage() {
    static QByteArray message;
    message.append(socket->readAll());
    qsizetype msgEndIndex = -1;
    while (true) {
        msgEndIndex = message.indexOf(QByteArrayView("||"), 0);
        if (msgEndIndex == -1) return; //The message is not complete. Save the portion that we got and do nothing with it.
        std::string completeMsgString = message.left(msgEndIndex).toStdString();

        message.slice(msgEndIndex+2); //Remove the previous message from the byte array.

        if (isMessageValid(completeMsgString))
        switch (completeMsgString[0]) {
        case 'W':
            emit waitingForPlayer();
            break;
        case 'B':
            emit battleStart(completeMsgString.substr(2,completeMsgString.length()));
            break;
        case 'R':
            emit battleFinish(completeMsgString[2], completeMsgString.substr(4,completeMsgString.length()-3));
            break;
        }
    }
}
