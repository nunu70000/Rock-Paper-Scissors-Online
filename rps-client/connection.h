#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtGlobal>
#include <QString>
#include <QHostAddress>

namespace Connection {
    extern quint16 port;
    extern QHostAddress ipAddress;
    extern QString username;
}
#endif // CONNECTION_H
