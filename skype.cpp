#include "skype.h"
#include "SignupSignin/authentication.h"
#include "mainwindow.h"
#include <QDebug>
#include "sox.h"
#include "webcam.h"
#include "updater.h"

Skype::Skype()
{
    Updater* updater = new Updater;
    updater->show();
    connect(updater, SIGNAL(endUpdate()), this, SLOT(authentication()));
}

void Skype::Connected(QString str)
{
    sox = new Sox;
    sox->moveToThread(&thread);
    thread.start();

    webCam = new WebCam;
    webCam->moveToThread(&threadWebCam);
    threadWebCam.start();

    Main = new MainWindow(Socket, str, sox, webCam);
    Main->show();
    connect(Main, SIGNAL(removeNoise()),
            sox, SLOT(removeNoise()));
    connect(sox, SIGNAL(sendSound(QByteArray)),
            Main, SLOT(sendSound(QByteArray)));
    connect(Main, SIGNAL(startRecord()),
            sox, SLOT(startRecord()));

    connect(Main, SIGNAL(startRecordVideo()),
            webCam, SLOT(startRecord()));
    connect(webCam, SIGNAL(sendCamera(QByteArray)),
            Main, SLOT(sendCamera(QByteArray)));

}

void Skype::authentication()
{
    Socket = new QTcpSocket(this);
    Authentication* auth = new Authentication(Socket, this);
    auth->show();
}
