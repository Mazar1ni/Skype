#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTime>
#include <QIcon>

class FriendWidget : public QWidget
{
    Q_OBJECT
public:
    FriendWidget(QString friendsId, QString l, QString em, QString f, QString ph,
                 QString st, QString um, QString in, QString idPar,
                 QString identNumber, bool isFr, QWidget *par = nullptr);

    void updateStatus(QString stat);
    void newUnreadMessages();
    void readUnreadMessages();
    void updateInfo(QString info);
    int getCountUnreadMessages() const;
    QString getLogin() const;
    QString getName() const;
    QIcon getProfileIcon() const;
    bool getCallStatus() const;
    void setCallStatus(bool value);
    bool getVideoStatus() const;
    void setVideoStatus(bool value);
    QTime getTimeCall() const;
    void downloadNewIcon(QString nameIcon);
    bool getIsFriend() const;
    void setIsFriend(bool value);
    bool getIsEnableInviteToFriend() const;
    void setIsEnableInviteToFriend(bool value);
    QString getIsAcceptFriendInvitation() const;
    void setIsAcceptFriendInvitation(const QString &value);
    QString getIsTryingCall() const;
    void setIsTryingCall(const QString &value);

public slots:
    void incrementTimeSec();

private slots:
    void updateIcon();

private:
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(FriendWidget*);
    void updateTimeCall(QString);

public:
    QString id;
    QString login;
    QString email;
    QString name;
    QString phone;
    QString status;
    int countUnreadMessages;
    QString iconName;
    QString idParent;
    QString identificationNumber;
    bool isFriend;
    bool isEnableInviteToFriend = true;
    QString isAcceptFriendInvitation = "false";
    QLabel* profileStatus;
    QLabel* profileName;
    QLabel* profileUnreadMessages;
    QLabel* profileIcon;
    QTime timeCall;
    QTimer* timer;
    QIcon icon;
    bool callStatus = false;
    bool videoStatus = false;
    QString isTryingCall = "false";
};

#endif // FRIENDWIDGET_H
