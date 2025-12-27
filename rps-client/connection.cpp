#include "connection.h"

Connection::Connection(QObject *parent)
    : QObject{parent}
{
}


void Connection::getConnectionInfo(MainWindow &w)
{
    port = w.getPort();
    quint32 ip = w.getIP();
    ipAddress = QHostAddress(ip);
    username = w.getUsername();
}

//See later if a destructor is needed
