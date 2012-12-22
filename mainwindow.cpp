#include "mainwindow.h"
#include "ui_mainwindow.h"

#define port 25

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket;
    this->setWindowTitle(qApp->applicationName());
}

void MainWindow::getIP(){
    QList< QHostAddress > addresses = QHostInfo::fromName( QHostInfo::localHostName() ).addresses();
        foreach ( const QHostAddress & a, addresses ){
            QString protocol = "???";
                switch ( a.protocol() ){
                    // anee IPv4
                    case QAbstractSocket::IPv4Protocol:
                        protocol = "IPv4";
                    break;
                    case QAbstractSocket::IPv6Protocol:
                        protocol = "IPv6";
                    break;
                    case QAbstractSocket::UnknownNetworkLayerProtocol:
                        protocol = "err";
                    break;
                }
                if(protocol == "IPv4"){
                    //address = a.toString();
                    break;
                }
        }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    url = ui->serverEdit->text();
    url.remove("smtp.");
    url.prepend(ui->userEdit->text() + "@");
    if (authorize()) {
        QMessageBox::information(this, qApp->applicationName(), tr("Авторизация успешна\nОтправляйте сообщение ;-)"),
                                 QMessageBox::Ok);
    } else {
        QMessageBox::critical(this, qApp->applicationName(), tr("Ошибка авторизации"), QMessageBox::Ok);
    }
}

bool MainWindow::authorize(){
    if (ui->serverEdit->text().isEmpty())
        return false;
    if (ui->userEdit->text().isEmpty())
        return false;
    if (ui->psswdEdit->text().isEmpty())
        return false;

    socket->connectToHost(ui->serverEdit->text(), (qint16)port);

    QByteArray arr;
    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "соединяемся" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }
    QByteArray array;
    array.append("EHLO 85.237.37.11\r\n");
    socket->write(array);

    arr.clear();

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "привет" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }

    socket->write("AUTH LOGIN\r\n");

    arr.clear();

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "авторизуемся" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }

    arr.clear();
    array.clear();
    array.append(ui->userEdit->text());
    array = array.toBase64();
    array.append("\r\n");
    socket->write(array);

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "отправили логин" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }

    arr.clear();
    array.clear();
    array.append(ui->psswdEdit->text());
    array = array.toBase64();
    array.append("\r\n");
    socket->write(array);

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "отправили пароль" << arr.data();
        if(arr.endsWith(".\r\n")) {
            if (arr.contains("successful")) {
                return true;
            }
            break;
        }
    }

    return false;
}

void MainWindow::on_pushButton_2_clicked()
{
    socket->write(qPrintable("MAIL FROM:" + url +"\r\n"));

    QByteArray arr;

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "FROM" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }

    socket->write(qPrintable("RCPT TO: " + ui->toEdit->text() + "\r\n"));

    arr.clear();

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "TO" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }

    socket->write(qPrintable("DATA\r\n"));

    arr.clear();

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "DATA" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }

    socket->write(qPrintable("from: " + url +" \r\n"));
    socket->write(qPrintable("to: " + ui->toEdit->text() + "\r\n"));
    socket->write(qPrintable("subject: " + ui->themeEdit->text() + "\r\n"));
    socket->write("\r\n");
    socket->write(qPrintable(ui->mailText->toPlainText() + "\r\n"));
    socket->write(".\r\n");
    socket->waitForBytesWritten(3000);

    arr.clear();

    while(socket->waitForReadyRead(1000))
    {
        arr.insert(arr.length(), socket->readAll());
        qDebug() << "SEND" << arr.data();
        if(arr.endsWith(".\r\n"))
            break;
    }
    if (arr.startsWith("250")) {
        QMessageBox::information(this, qApp->applicationName(), tr("Сообщение отправлено :-)"), QMessageBox::Ok);
    } else {
        QMessageBox::critical(this, qApp->applicationName(), tr("Что-то пошло не так..\nНе удалось отправить сообщение"), QMessageBox::Ok);
    }
}
