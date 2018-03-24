#include "skype.h"
#include "SignupSignin/authentication.h"
#include "mainwindow.h"
#include <QDebug>
#include "audio.h"
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
    audio = new Audio;
    audio->moveToThread(&thread);
    thread.start();

    webCam = new WebCam;
    webCam->moveToThread(&threadWebCam);
    threadWebCam.start();

    Main = new MainWindow(Socket, str, audio, webCam);
    Main->show();
    connect(audio, SIGNAL(sendSound(QByteArray)),
            Main, SLOT(sendSound(QByteArray)));
    connect(Main, SIGNAL(startRecord()),
            audio, SLOT(startRecord()));
    connect(Main, SIGNAL(connectSoundServer(QString, QString)),
            audio, SLOT(connectServer(QString, QString)));
    connect(audio, SIGNAL(outOfTheRoom()),
            Main, SLOT(outOfTheRoom()));
    connect(audio, SIGNAL(connectedAudio()),
            Main, SLOT(connectedAudio()));

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
