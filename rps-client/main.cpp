#include "mainwindow.h"
#include "connection.h"
#include "rpsgameclient.h"

#include <QApplication>
#include <QTcpSocket>
#include <qpushbutton.h>

namespace Connection {
    quint16 port;
    QHostAddress ipAddress;
    QString username;
}

QTcpSocket * login() { //note: can allow connecting using hostname, not just IP.
    QTcpSocket * socket = new(std::nothrow) QTcpSocket;
    if (socket == nullptr) return nullptr;
    socket->connectToHost(Connection::ipAddress, Connection::port); //emits QAbstractSocket::connected()
    return socket;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    rpsGameClient client;
    QObject::connect(&w, &MainWindow::hitConnectButton, &client, &rpsGameClient::connect);
    QObject::connect(&client, &rpsGameClient::connectionError, &w, &MainWindow::connectionError);
    QObject::connect(&client, &rpsGameClient::waitingForPlayer, &w, &MainWindow::enterWaiting);
    QObject::connect(&client, &rpsGameClient::battleStart, &w, &MainWindow::enterBattle);
    QObject::connect(&w, &MainWindow::attack, &client, &rpsGameClient::sendAttack);
    QObject::connect(&client, &rpsGameClient::battleFinish, &w, &MainWindow::enterResults);
    QObject::connect(&w, &MainWindow::rematch, &client, &rpsGameClient::sendRematch);
    QObject::connect(&w, &MainWindow::quit, &client, &rpsGameClient::disconnect);
    w.show();

    return a.exec();
}
