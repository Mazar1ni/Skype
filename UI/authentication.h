#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QWidget>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QKeyEvent>

class SettingsConnection;
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
    void OpenSettings();
    void Connect();
    void slotConnected();
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError err);

protected:
  virtual void  keyPressEvent(QKeyEvent *event);

private:
    Skype* Parent;
    QTcpSocket* Socket;
    QTcpSocket* AudioSocket;
    QLineEdit* textLogin;
    QLineEdit* textPass;
    QPushButton* btnConnect;
    QPushButton* btnSettings;
    SettingsConnection* Settings;
    const QString FileName = "SettingsConnection.txt";

};

#endif // AUTHENTICATION_H
