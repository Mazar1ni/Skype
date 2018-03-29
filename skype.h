#ifndef SKYPE_H
#define SKYPE_H

#include <QTcpSocket>
#include <QThread>
#include <QTranslator>

class MainWindow;
class Audio;
class WebCam;

class Skype : public QObject
{
    Q_OBJECT

public:
    Skype();

public slots:
    void Connected(QString str);
    void authentication();

private:
    QTcpSocket* Socket;
    MainWindow* Main;
    Audio* audio;
    QThread thread;
    QThread threadWebCam;
    WebCam* webCam;
    QTranslator translator;

};

#endif // SKYPE_H
