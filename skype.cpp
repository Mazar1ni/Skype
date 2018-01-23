#include "skype.h"
#include "UI/authentication.h"
#include "mainwindow.h"
#include <QDebug>
#include "sox.h"

Skype::Skype()
{
//    Socket = new QTcpSocket(this);
//    Main = new MainWindow(Socket);
//    Main->show();

    Socket = new QTcpSocket(this);
    Authentication* auth = new Authentication(Socket, this);
    auth->show();
}

void Skype::Connected(QString str)
{
    sox = new Sox(Socket);
    sox->moveToThread(&thread);
    thread.start();

    Main = new MainWindow(Socket, str, sox);
    Main->show();
    connect(Main, SIGNAL(removeNoise()),
            sox, SLOT(removeNoise()));
    connect(sox, SIGNAL(sendSound(QByteArray)),
            Main, SLOT(sendSound(QByteArray)));
    connect(Main, SIGNAL(startRecord()),
            sox, SLOT(startRecord()));
    connect(Main, SIGNAL(stopRecord()),
            sox, SLOT(stopRecord()));
}
