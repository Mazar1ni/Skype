#include "friendwidget.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

FriendWidget::FriendWidget(QString friendsId, QString l, QString em, QString f,
                           QString ph, QString st, QString um, QWidget *parent) :
    QWidget(parent), id(friendsId), login(l), email(em), name(f),
    phone(ph), status(st), countUnreadMessages(um.toInt())
{
    setMaximumSize(200, 60);

    icon = QIcon(":/Icons/standart_icon.png");

    profileIcon = new QLabel(this);
    profileIcon->setPixmap(icon.pixmap(64, 64));

    QLabel* profileName = new QLabel(this);
    profileName->setText(name);
    profileName->move(70, 15);

    profileStatus = new QLabel(this);
    profileStatus->setText(status);
    profileStatus->move(70, 35);

    profileUnreadMessages = new QLabel(this);
    profileUnreadMessages->setMargin(3);
    profileUnreadMessages->setStyleSheet("QLabel { background-color : red;}");
    profileUnreadMessages->move(160, 10);
    profileUnreadMessages->setFixedWidth(25);
    profileUnreadMessages->setVisible(false);

    if(countUnreadMessages != 0)
    {
        profileUnreadMessages->setText(QString::number(countUnreadMessages));
        profileUnreadMessages->setVisible(true);
    }
    timeCall.setHMS(0, 0, 0);

    timer = new QTimer;
}

void FriendWidget::updateStatus(QString stat)
{
    status = stat;
    profileStatus->setText(status);
}

void FriendWidget::newUnreadMessages()
{
    countUnreadMessages++;
    profileUnreadMessages->setText(QString::number(countUnreadMessages));
    profileUnreadMessages->setVisible(true);
}

void FriendWidget::readUnreadMessages()
{
    countUnreadMessages--;
    if(countUnreadMessages == 0)
    {
        profileUnreadMessages->clear();
        profileUnreadMessages->setVisible(false);
    }
}

void FriendWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        clicked(this);
    }
}

bool FriendWidget::getVideoStatus() const
{
    return videoStatus;
}

void FriendWidget::setVideoStatus(bool value)
{
    videoStatus = value;
}

QTime FriendWidget::getTimeCall() const
{
    return timeCall;
}

void FriendWidget::incrementTimeSec()
{
    timeCall = timeCall.addSecs(1);
    updateTimeCall(timeCall.toString("hh:mm:ss"));
}

bool FriendWidget::getCallStatus() const
{
    return callStatus;
}

void FriendWidget::setCallStatus(bool value)
{
    if(value == true)
    {
        connect(timer, SIGNAL(timeout()), this, SLOT(incrementTimeSec()));
        timer->start(1000);
    }
    else
    {
        timer->stop();
    }
    callStatus = value;
}

QString FriendWidget::getName() const
{
    return name;
}

QIcon FriendWidget::getProfileIcon() const
{
    return icon;
}

QString FriendWidget::getLogin() const
{
    return login;
}

int FriendWidget::getCountUnreadMessages() const
{
    return countUnreadMessages;
}
