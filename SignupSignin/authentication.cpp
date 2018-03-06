#include "authentication.h"
#include "settingsconnection.h"
#include "skype.h"
#include "registration.h"
#include <QLabel>
#include <QFile>
#include <QHostAddress>
#include <QMessageBox>
#include <qDebug>
#include <QThread>
#include <QEventLoop>
#include <QTimer>

Authentication::Authentication(QTcpSocket *soc, Skype *parent) : Parent(parent), Socket(soc)
{
    // оформление окна
    this->setFixedSize(400, 150);

    // инициализаци€ и оформление ввода Login
    textLogin = new QLineEdit(this);
    textLogin->resize(200, 20);
    textLogin->move(20, 20);
    textLogin->setText("Mazarini");
    QLabel* labelLogin = new QLabel("Login:", this);
    labelLogin->move(20, 5);

    // инициализаци€ и оформление ввода Password
    textPass = new QLineEdit(this);
    textPass->resize(200, 20);
    textPass->move(20, 60);
    QLabel* labelPass = new QLabel("Password:", this);
    textPass->setText("Sanhez123");
    labelPass->move(20, 45);

    // инииализаци€ и оформление кнопки Connect
    btnConnect = new QPushButton(this);
    btnConnect->resize(100, 30);
    btnConnect->move(120, 100);
    btnConnect->setText("Sign in");
    connect(btnConnect, SIGNAL(clicked(bool)), this, SLOT(Connect()));

    QLabel* textLabel = new QLabel(this);
    textLabel->move(230, 110);
    textLabel->setText("or");

    // инициализаци€ и оформление текста перенаправл€ющего на сайт
    QPushButton* LinkReginstration = new QPushButton(this);
    LinkReginstration->move(250, 100);
    LinkReginstration->resize(110, 30);
    LinkReginstration->setStyleSheet("border: none; color: blue; font: bold 14p;");
    LinkReginstration->setText("Create an account");
    connect(LinkReginstration, &QPushButton::clicked, [this](){
        Registration* registrationWidget = new Registration;
        registrationWidget->setAttribute(Qt::WA_ShowModal, true);
        registrationWidget->show();
    });

    // инииализаци€ и оформление кнопки настройки подключени€ к серверу
    btnSettings = new QPushButton(this);
    btnSettings->resize(30, 30);
    btnSettings->move(350, 40);
    btnSettings->setIcon(QIcon("://Icons/Settings.png"));
    btnSettings->setIconSize(QSize(30, 30));
    connect(btnSettings, SIGNAL(clicked(bool)), this, SLOT(OpenSettings()));

    // вызвать сигнал в случае правильного ввода парол€
    connect(this, SIGNAL(ClientConnected(QString)),
            Parent, SLOT(Connected(QString)));

}

void Authentication::slotSendToServer()
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << textLogin->text() + "!" + textPass->text() + "/1/";

    Socket->write(arrBlock);
}

void Authentication::OpenSettings()
{
    Settings = new SettingsConnection;
    Settings->setAttribute(Qt::WA_ShowModal, true);
    Settings->show();
}

void Authentication::Connect()
{
    if(!Socket->isOpen())
    {
        // открытие файла настроек дл€ получени€ IP и Port
        QFile in(FileName);
        QString text;
        int pos;
        if(in.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&in);
            text = stream.readAll();
            pos = text.indexOf(":");

            // инициализаи€ сокета
            //QHostAddress(text.left(pos))
            Socket->connectToHost("localhost", (qint16)text.mid(pos+1).toInt());
            in.close();

            connect(Socket, SIGNAL(connected()), SLOT(slotConnected()));
            connect(Socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
            connect(Socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
        }
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    slotSendToServer();
}

void Authentication::slotConnected()
{

}

void Authentication::slotReadyRead()
{
    QDataStream in(Socket);

    QString str;
    in >> str;

    // проверка правильный ли логин и пароль
    if(str.indexOf("/1/") != -1)
    {
        str.remove(0, 3);
        ClientConnected(str);
        deleteLater();
    }
    else if(str == "/0/")
    {
        QMessageBox::critical(NULL,QObject::tr("Error"), "Username or password is invalid!");
    }
    else if(str == "/-1/")
    {
        QMessageBox::critical(NULL,QObject::tr("Error"), "This user is already in the network!");
    }
}

void Authentication::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                         "The host was not found." :
                         err == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         err == QAbstractSocket::ConnectionRefusedError ?
                         "The connection was refused." :
                         QString(Socket->errorString())
                        );
    qDebug() << strError;
}

void Authentication::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)
    {
        Connect();
    }
}
