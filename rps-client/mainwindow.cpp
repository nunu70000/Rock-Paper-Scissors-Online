#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"
#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(this->ui->connectButton, &QPushButton::clicked, this, &MainWindow::updateConnection);
    QObject::connect(this->ui->disconnectButton, &QPushButton::clicked, this, &MainWindow::pressedQuit);
    QObject::connect(this->ui->forfeitButton, &QPushButton::clicked, this, &MainWindow::pressedQuit);
    QObject::connect(this->ui->quitPushButton, &QPushButton::clicked, this, &MainWindow::pressedQuit);
    QObject::connect(this->ui->rockRadioButton, &QRadioButton::clicked, this, &MainWindow::selectRockAttack);
    QObject::connect(this->ui->paperRadioButton, &QRadioButton::clicked, this, &MainWindow::selectPaperAttack);
    QObject::connect(this->ui->scissorsRadioButton, &QRadioButton::clicked, this, &MainWindow::selectScissorsAttack);
    QObject::connect(this->ui->confirmAttackButton, &QPushButton::clicked, this, &MainWindow::confirmAttack);
    QObject::connect(this->ui->continuePushButton, &QPushButton::clicked, this, &MainWindow::waitingRematch);
    this->setWindowTitle("Rock Paper Scissors: Online");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateConnection() {
    ui->formErrorLabel->setText("Connecting...");
    Connection::port = ui->portLineEdit->text().toShort();
    Connection::ipAddress = QHostAddress(this->ui->serverIPLineEdit->text());
    Connection::username =  ui->usernameLineEdit->text();
    ui->connectButton->setEnabled(false);
    ui->connectButton->setText("Connecting");
    emit hitConnectButton();
}

void MainWindow::connectionError(QAbstractSocket::SocketError error) {
    if (error == QAbstractSocket::RemoteHostClosedError) enterLogin();
    else {
        ui->formErrorLabel->setText("<html><head/><body><p><span style=\"color:#ff0000;\">Cannot connect to server</span></p></body></html>");
        ui->connectButton->setEnabled(true);
        ui->connectButton->setText("Connect");
    }
}

void MainWindow::enterLogin() {
    ui->pages->setCurrentIndex(0);
    ui->formErrorLabel->setText("Disconnected from server");
    ui->connectButton->setEnabled(true);
    ui->connectButton->setText("Connect");
}

void MainWindow::enterWaiting() {
    ui->pages->setCurrentIndex(1);
}

void MainWindow::enterBattle(std::string opponentName) {
    //Potential issue to this: it may be possible for somebody to do funny HTML stuff with this
    QString labelString = "<html><head/><body><p>Your opponent is <span style=\" font-weight:700;\">";
    labelString.append(opponentName);
    labelString.append("</span>.</p></body></html>");
    ui->opponentNameLabel->setText(labelString);
    ui->attackDescriptionTextBrowser->setText("Select a weapon for details.");
    ui->rockRadioButton->setEnabled(true);
    ui->paperRadioButton->setEnabled(true);
    ui->scissorsRadioButton->setEnabled(true);
    ui->forfeitButton->setEnabled(true);
    ui->confirmAttackButton->setText("STRIKE!");
    ui->pages->setCurrentIndex(2);
}

void MainWindow::enterResults(char result, std::string opponentAttack) {
    QString resultText = "<html><head/><body><p><span style=\" font-size:72pt; font-weight:700; font-style:italic;\">";
    switch (result) {
    case 'W':
        resultText.append("Victory!");
        break;
    case 'L':
        resultText.append("Defeat!");
        break;
    case 'F':
        resultText.append("Victory");
        break;
    default:
        resultText.append("Draw");
    }
    resultText.append("</span></p></body></html>");
    QString opponentChoiceText = "<html><head/><body><p><span style=\" font-size:22pt;\">";
    if (result == 'F') {
        opponentChoiceText.append("Your opponent forfeited.");
    }
    else {
        opponentChoiceText.append("Your opponent's choice of attack was ");
        QString o_attack;
        o_attack.append(opponentAttack);
        opponentChoiceText.append(o_attack.toLower());
    }
    opponentChoiceText.append("</span></p></body></html>");
    ui->continuePushButton->setText("Rematch");
    ui->continuePushButton->setEnabled(true);
    ui->quitPushButton->setEnabled(true);
    ui->opponentResultLabel->setText(opponentChoiceText);
    ui->resultLabel->setText(resultText);
    ui->pages->setCurrentIndex(3);
}

void MainWindow::selectRockAttack() {
    //Theres probably a better way to do this (other than just putting it into a file and using that). But that's not necessary to look at now.
    ui->attackDescriptionTextBrowser->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                                              "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">"
                                              "p, li { white-space: pre-wrap; }"
                                              "hr { height: 1px; border-width: 0; }"
                                              "li.unchecked::marker { content: \"\\2610\"; }"
                                              "li.checked::marker { content: \"\\2612\"; }"
                                              "</style></head><body style=\" font-family:'DejaVu LGC Sans'; font-size:12pt; font-weight:400; font-style:normal;\">"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "<span style=\" font-weight:700;\">Rock</span><span style=\" font-weight:700; font-style:italic;\"> </span><span style=\" font-style:italic;\">(Effective against: Scissors, Weak to: Paper)</span></p>"
                                              "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:700;\"><br /></p>"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "A brutal weapon of immense strength, the rock has been utilized in the destruction of innumerable pairs of scissors.</p>"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "Its raw strength, however, is ineffectual in a battle against paper. </p></body></html>""");
    ui->confirmAttackButton->setEnabled(true);
}

void MainWindow::selectPaperAttack() {
    ui->attackDescriptionTextBrowser->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                                              "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">"
                                              "p, li { white-space: pre-wrap; }"
                                              "hr { height: 1px; border-width: 0; }"
                                              "li.unchecked::marker { content: \"\\2610\"; }"
                                              "li.checked::marker { content: \"\\2612\"; }"
                                              "</style></head><body style=\" font-family:'DejaVu LGC Sans'; font-size:12pt; font-weight:400; font-style:normal;\">"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "<span style=\" font-weight:700;\">Paper</span><span style=\" font-weight:700; font-style:italic;\"> </span><span style=\" font-style:italic;\">(Effective against: Rock, Weak to: Scissors)</span></p>"
                                              "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:700;\"><br /></p>"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "Thin and flexible, paper's unique malleability lends to its capability to smother the rock, making it a weapon of choice against rock users.</p>"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "This attribute, however, renders it susceptible to annilihation by the blades of a pair of scissors.</p></body></html>");
    ui->confirmAttackButton->setEnabled(true);
}

void MainWindow::selectScissorsAttack() {
    ui->attackDescriptionTextBrowser->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                                              "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">"
                                              "p, li { white-space: pre-wrap; }"
                                              "hr { height: 1px; border-width: 0; }"
                                              "li.unchecked::marker { content: \"\\2610\"; }"
                                              "li.checked::marker { content: \"\\2612\"; }"
                                              "</style></head><body style=\" font-family:'DejaVu LGC Sans'; font-size:12pt; font-weight:400; font-style:normal;\">"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "<span style=\" font-weight:700; font-style:italic;\">Scissors </span><span style=\" font-style:italic;\">(Effective against: Paper, Weak to: Rock)</span></p>"
                                              "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:700;\"><br /></p>"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "A marvel of engineering designed explicitly to tear through paper. </p>"
                                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                              "A sharp pair of scissors will trivialize any battle against a paper user, but it can just as quickly turn fatal in an encounter with a rock.</p></body></html>");
    ui->confirmAttackButton->setEnabled(true);
}

void MainWindow::confirmAttack() {
    ui->confirmAttackButton->setEnabled(false);
    ui->rockRadioButton->setEnabled(false);
    ui->paperRadioButton->setEnabled(false);
    ui->scissorsRadioButton->setEnabled(false);
    ui->forfeitButton->setEnabled(false);
    ui->confirmAttackButton->setText("Attack sent. Waiting for server.");
    if (ui->rockRadioButton->isChecked()) {
        emit attack('R');
        ui->rockRadioButton->setChecked(false);
    }
    else if (ui->paperRadioButton->isChecked()) {
        emit attack('P');
        ui->paperRadioButton->setChecked(false);
    }
    else {
        emit attack('S');
        ui->scissorsRadioButton->setChecked(false);
    }
}

void MainWindow::waitingRematch() {
    ui->continuePushButton->setEnabled(false);
    ui->continuePushButton->setText("Rematch sent. Waiting for opponent.");
    ui->quitPushButton->setEnabled(false);
    emit rematch();
}

void MainWindow::pressedQuit() {
    emit quit();
    enterLogin();
}






















