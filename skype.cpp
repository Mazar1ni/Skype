#include "skype.h"
#include "SignupSignin/authentication.h"
#include "mainwindow.h"
#include <QDebug>
#include "audio.h"
#include "webcam.h"
#include "updater.h"
#include <QSettings>

Skype::Skype()
{
    // установка языка указанного в настройках (поумолчанию english)
    QSettings setting("settings.conf", QSettings::IniFormat);
    setting.beginGroup("GeneralSettings");
    translator.load(":/Translations/" + setting.value("Language").toString() + ".qm");
    qApp->installTranslator(&translator);
    setting.endGroup();

    // проверка обновлений и их загрузка
    Updater* updater = new Updater;
    updater->show();
    connect(updater, SIGNAL(endUpdate()), this, SLOT(authentication()));
}

void Skype::Connected(QString str)
{
    // создание класса работающего со звуком
    audio = new Audio;
    audio->moveToThread(&thread);
    thread.start();

    // создание класса работающий с веб-камерой
    webCam = new WebCam;
    webCam->moveToThread(&threadWebCam);
    threadWebCam.start();

    // создание главного класса
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
    connect(Main, SIGNAL(updateSettings()),
            audio, SLOT(updateSettings()));

    connect(Main, SIGNAL(startRecordVideo()),
            webCam, SLOT(startRecord()));
    connect(webCam, SIGNAL(sendCamera(QByteArray)),
            Main, SLOT(sendCamera(QByteArray)));
    connect(Main, SIGNAL(updateSettings()),
            webCam, SLOT(updateSettings()));

}

void Skype::authentication()
{
    Socket = new QTcpSocket(this);
    Authentication* auth = new Authentication(Socket, this);
    auth->show();
}
