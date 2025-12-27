#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;

signals:
    void hitConnectButton();
    void hitDisconnect();
    void attack(char attack);
    void rematch();
    void quit();

public slots:
    void connectionError(QAbstractSocket::SocketError error);
    void enterLogin();
    void enterWaiting();
    void enterBattle(std::string opponentName);
    void enterResults(char result, std::string opponentAttack);

private slots:
    void selectRockAttack();
    void selectPaperAttack();
    void selectScissorsAttack();
    void confirmAttack();
    void updateConnection();
    void waitingRematch();
    void pressedQuit();
};
#endif // MAINWINDOW_H
