#include "friendwidget.h"
#include "filetransfer.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QDir>

FriendWidget::FriendWidget(QString friendsId, QString l, QString em, QString f, QString ph, QString st,
                           QString um, QString in, QString idPar, QString identNumber, bool isFr, QWidget *par) :
    QWidget(par), id(friendsId), login(l), email(em), name(f),
    phone(ph), status(st), countUnreadMessages(um.toInt()), iconName(in), idParent(idPar),
    identificationNumber(identNumber), isFriend(isFr)
{
    setFixedSize(200, 60);

    if(iconName != "standart_icon.png")
    {
        QDir().mkdir("IconFriends");
        if(!QFile::exists("IconFriends/" + id + "!" + iconName))
        {
            FileTransfer* fileTransfer = new FileTransfer(idPar, identificationNumber, "downloadFriendIcon", id + "!" + iconName);
            fileTransfer->start();
            connect(fileTransfer, SIGNAL(updateIconFriend()), this, SLOT(updateIcon()));
        }
        else
        {
            icon = QIcon("IconFriends/" + id + "!" + iconName);
        }
    }
    else
    {
        icon = QIcon(":/Icons/standart_icon.png");
    }

    profileIcon = new QLabel(this);
    profileIcon->setPixmap(icon.pixmap(64, 64));
    profileIcon->setMinimumSize(64, 64);

    profileName = new QLabel(this);
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

void FriendWidget::updateInfo(QString info)
{
    if(info.indexOf("/name/") != -1)
    {
        info.remove("/name/");
        name = info;
        profileName->setText(name);
    }

    if(info.indexOf("/email/") != -1)
    {
        info.remove("/email/");
        email = info;
    }

    if(info.indexOf("/phone/") != -1)
    {
        info.remove("/phone/");
        phone = info;
    }
}

void FriendWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        clicked(this);
    }
}

QString FriendWidget::getIsTryingCall() const
{
    return isTryingCall;
}

void FriendWidget::setIsTryingCall(const QString &value)
{
    isTryingCall = value;
}

void FriendWidget::setIsFriend(bool value)
{
    isFriend = value;
}

void FriendWidget::setIsAcceptFriendInvitation(const QString &value)
{
    isAcceptFriendInvitation = value;
    if(value == "true")
    {
        this->setStyleSheet("background-color: #fc3;");
    }
    else
    {
        this->setStyleSheet("background-color: transparent;");
    }
}

QString FriendWidget::getIsAcceptFriendInvitation() const
{
    return isAcceptFriendInvitation;
}

bool FriendWidget::getIsEnableInviteToFriend() const
{
    return isEnableInviteToFriend;
}

void FriendWidget::setIsEnableInviteToFriend(bool value)
{
    isEnableInviteToFriend = value;
}

bool FriendWidget::getIsFriend() const
{
    return isFriend;
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

void FriendWidget::downloadNewIcon(QString nameIcon)
{
    iconName = nameIcon;
    FileTransfer* fileTransfer = new FileTransfer(id, identificationNumber, "downloadFriendIcon", id + "!" + nameIcon);
    fileTransfer->start();
    connect(fileTransfer, SIGNAL(updateIconFriend()), this, SLOT(updateIcon()));
}

void FriendWidget::updateIcon()
{
    icon = QIcon("IconFriends/" + id + "!" + iconName);
    profileIcon->setPixmap(icon.pixmap(64, 64));
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
