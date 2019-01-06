#include "authentication.h"
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

    QString qss = ("QPushButton{"
                              "font-weight: 700;"
                              "text-decoration: none;"
                              "padding: .5em 2em;"
                              "outline: none;"
                              "border: 2px solid;"
                              "border-radius: 1px;"
                            "} "
                            "QPushButton:!hover { background: rgb(255,255,255); }");

    // инициализация и оформление ввода Login
    textLogin = new QLineEdit(this);
    textLogin->resize(200, 20);
    textLogin->move(20, 20);
    QLabel* labelLogin = new QLabel(tr("Login:"), this);
    labelLogin->move(20, 5);

    // инициализация и оформление ввода Password
    textPass = new QLineEdit(this);
    textPass->resize(200, 20);
    textPass->move(20, 60);
    textPass->setEchoMode(QLineEdit::Password);
    textPass->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
    QLabel* labelPass = new QLabel(tr("Password:"), this);
    labelPass->move(20, 45);

    // инииализация и оформление кнопки Connect
    btnConnect = new QPushButton(this);
    btnConnect->resize(100, 40);
    btnConnect->move(120, 100);
    btnConnect->setStyleSheet(qss);
    btnConnect->setText(tr("Sign in"));
    connect(btnConnect, SIGNAL(clicked(bool)), this, SLOT(Connect()));

    QLabel* textLabel = new QLabel(this);
    textLabel->move(230, 115);
    textLabel->setText(tr("or"));

    // инициализация и оформление кнопки регистрации
    QPushButton* LinkReginstration = new QPushButton(this);
    LinkReginstration->move(250, 100);
    LinkReginstration->resize(110, 40);
    LinkReginstration->setStyleSheet("border: none; color: blue; font: bold 14p;");
    LinkReginstration->setText(tr("Create an account"));
    connect(LinkReginstration, &QPushButton::clicked, [this](){
        Registration* registrationWidget = new Registration;
        registrationWidget->setAttribute(Qt::WA_ShowModal, true);
        registrationWidget->show();
    });

    // вызвать сигнал в случае правильного ввода пароля
    connect(this, SIGNAL(ClientConnected(QString)),
            Parent, SLOT(Connected(QString)));

}

// слот для отправки сообщений серверу
void Authentication::slotSendToServer()
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << textLogin->text() + "!" + textPass->text() + "/1/";

    Socket->write(arrBlock);
}

void Authentication::Connect()
{
    if(isConnect == true)
    {
        return;
    }
    // если сервер уже запущен перезапустить его
    if(!Socket->isOpen())
    {
        Socket->connectToHost("185.146.157.27", 7070);
        connect(Socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    slotSendToServer();
    isConnect = true;
}

// слот для приема сообщение от сервера
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
        QMessageBox::critical(NULL,tr("Error"), tr("Username or password is invalid!"));
    }
    else if(str == "/-1/")
    {
        QMessageBox::critical(NULL,tr("Error"), tr("This user is already in the network!"));
    }
    isConnect = false;
}

// отлавливаем нажатие Enter
void Authentication::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)
    {
        Connect();
    }
}
