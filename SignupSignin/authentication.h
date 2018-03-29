#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QWidget>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>

class Skype;

class Authentication : public QWidget
{
    Q_OBJECT
public:
    explicit Authentication(QTcpSocket *soc, Skype *parent = nullptr);

    void slotSendToServer();

signals:
    void ClientConnected(QString str);

private slots:
    void Connect();
    void slotReadyRead();

protected:
  virtual void  keyPressEvent(QKeyEvent *event);

private:
    Skype* Parent;
    QTcpSocket* Socket;
    QLineEdit* textLogin;
    QLineEdit* textPass;
    QPushButton* btnConnect;
    QPushButton* btnSettings;
    bool isConnect = false;

};

#endif // AUTHENTICATION_H
