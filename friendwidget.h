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
    FriendWidget(QString friendsId, QString l, QString em, QString f, QString ph, QString st, QString um, QWidget *parent = nullptr);

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

public slots:
    void incrementTimeSec();

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
    QLabel* profileStatus;
    QLabel* profileName;
    QLabel* profileUnreadMessages;
    QLabel* profileIcon;
    QTime timeCall;
    QTimer* timer;
    QIcon icon;
    bool callStatus = false;
    bool videoStatus = false;
    int countUnreadMessages = 0;
};

#endif // FRIENDWIDGET_H
