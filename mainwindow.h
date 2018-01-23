#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QBoxLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QtMultimedia/QAudioOutput>
#include <QIODevice>
#include <QScrollArea>

class Sox;
class FriendWidget;
class Message;
class QMediaPlayer;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QTcpSocket* Sock, QString str, Sox* Sox, QWidget *parent = nullptr);

    void SlotSendToServer(QString str);
    void gettingFriends(QString str);
    void createSettingRoomWidget();
    void upCalling(QString name, QString pass);
    void noUpCalling(QString name, QString pass);

signals:
    void startRecord();
    void output(QByteArray buffer);
    void stopRecord();
    void removeNoise();

public slots:
    void sendSound(QByteArray buff);

private slots:
    void CreateRoom();
    void OpenRoom();
    void SlotReadyRead();
    void SlotSendAudioToServer();
    void clickedFriendWidget(FriendWidget* friendW);
    void clickedCallButton();
    void clickedSendMessageButton();

private:

    QString id;
    QString login;
    QString email;
    QString fam;
    QString name;
    QString otch;
    QString phone;

    QVBoxLayout* messageVBox;
    QLineEdit* lineMessage;
    QTcpSocket* Socket;
    QVBoxLayout* friendsVBox;
    QLineEdit* nameRoom;
    QLineEdit* passRoom;
    QVBoxLayout* rightVBox;
    QWidget* settingRoom;
    QWidget* mainScreenWithButtons;
    FriendWidget* friendInf = nullptr;
    QList<FriendWidget*> friendWidgets;

    QAudioOutput* AudioOutput;
    QAudioFormat Format;
    QIODevice* DeviceOutput = nullptr;

    bool isCreatedRoom = false;
    bool isOpenedRoom = false;

    Sox* sox;

    QMediaPlayer* player;

    QList<Message*> messageWidgets;

    QScrollArea *scrollarea;

};

#endif // MAINWINDOW_H
