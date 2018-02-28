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
#include <QDate>
#include <QCloseEvent>
#include <QLabel>

class Sox;
class FriendWidget;
class Message;
class QMediaPlayer;
class WebCam;
class ProfileWidget;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QTcpSocket* Sock, QString str, Sox* Sox, WebCam* wb, QWidget *parent = nullptr);

    void SlotSendToServer(QString str);
    void gettingFriends(QString str, QString mode);
    void addRecent(QString id);
    void createSettingRoomWidget();
    void upCalling(QString name, QString pass);
    void noUpCalling(QString name, QString pass);
    void cleanLayout(QLayout *layout);
    void createCallWidget();
    void createMainFriendWidget();
    void closeEvent(QCloseEvent *event);
    void imgWin(QByteArray buff);
    void turnVideoBroadcast(int idSender, bool onOff);
    void updateInfo(QString info);

signals:
    void startRecord();
    void output(QByteArray buffer);
    void stopRecord();
    void removeNoise();
    void startRecordVideo();

public slots:
    void sendSound(QByteArray buff);
    void sendCamera(QByteArray buff);

private slots:
    void CreateRoom();
    void OpenRoom();
    void SlotReadyRead();
    void clickedFriendWidget(FriendWidget* friendW);
    void clickedCallButton();
    void clickedSendMessageButton();
    void requestNewMessage(int value);
    void endCall();
    void clickedMicroButton();
    void clickedVideoButton();
    void clickedProfileWidget();

private:
    QString id;
    QString login;
    QString email;
    QString name;
    QString phone;
    QString identificationNumber;
    QString iconName;

    ProfileWidget* profileWidget;
    QVBoxLayout* messageVBox;
    QLineEdit* lineMessage;
    QTcpSocket* Socket;
    QVBoxLayout* friendsVBox;
    QVBoxLayout* recentVBox;
    QLineEdit* nameRoom;
    QLineEdit* passRoom;
    QVBoxLayout* rightVBox;
    QWidget* settingRoom;
    QWidget* mainScreenWithButtons;
    FriendWidget* friendInf = nullptr;
    QList<FriendWidget*> friendWidgets;
    QList<FriendWidget*> recentWidgets;

    QAudioOutput* AudioOutput;
    QAudioFormat Format;
    QIODevice* DeviceOutput = nullptr;

    bool isCreatedRoom = false;
    bool isOpenedRoom = false;

    bool isScrolling;
    bool isMicro = true;
    bool isVideo = false;

    QString numberBlockMessage = "1";

    Sox* sox;
    QDate Date;

    QMediaPlayer* player;

    QList<Message*> messageWidgets;

    QScrollArea *scrollarea;
    WebCam* webCam;
    QLabel* cam;
    QByteArray video;

    QHBoxLayout* camAndIconLayout;
    QLabel* iconFriend;
    QLabel* profileIcon;

};

#endif // MAINWINDOW_H
