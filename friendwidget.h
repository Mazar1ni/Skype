#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include <QWidget>
#include <QLabel>

class FriendWidget : public QWidget
{
    Q_OBJECT
public:
    FriendWidget(QString friendsId, QString l, QString em, QString f, QString ph, QString st, QString um, QWidget *parent = nullptr);

    void updateStatus(QString stat);
    void newUnreadMessages();
    void readUnreadMessages();
    int getCountUnreadMessages() const;

private:
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(FriendWidget*);

public:
    QString id;
    QString login;
    QString email;
    QString fio;
    QString phone;
    QString status;
    QLabel* profileStatus;
    QLabel* profileUnreadMessages;
    int countUnreadMessages = 0;
};

#endif // FRIENDWIDGET_H
