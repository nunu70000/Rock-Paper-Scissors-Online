#ifndef RPSGAMECLIENT_H
#define RPSGAMECLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>

class rpsGameClient : public QObject
{
    Q_OBJECT
public:
    explicit rpsGameClient(QObject *parent = nullptr);
    QTcpSocket * socket;

public slots:
    void connect();
    void sendAttack(char attack);
    void sendRematch();
    void disconnect();

private:
    static bool isMessageValid(std::string);
    void sendName();
    void getMessage();

private slots:
    void tcpSocketError(QAbstractSocket::SocketError error);

signals:
    void connectionError(QAbstractSocket::SocketError error);
    void waitingForPlayer();
    void battleStart(std::string opponentName);
    void battleFinish(char result, std::string opponentAttack);
};

#endif // RPSGAMECLIENT_H
