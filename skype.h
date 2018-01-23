#ifndef SKYPE_H
#define SKYPE_H

#include <QTcpSocket>
#include <QThread>

class MainWindow;
class Sox;

class Skype : public QObject
{
    Q_OBJECT

public:
    Skype();

public slots:
    void Connected(QString str);

private:
    QTcpSocket* Socket;
    MainWindow* Main;
    Sox* sox;
    QThread thread;

};

#endif // SKYPE_H
