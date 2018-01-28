#include "friendwidget.h"
#include <QMouseEvent>
#include <QIcon>
#include <QMessageBox>
#include <QDebug>

FriendWidget::FriendWidget(QString friendsId, QString l, QString em, QString f,
                           QString ph, QString st, QString um, QWidget *parent) :
    QWidget(parent), id(friendsId), login(l), email(em), fio(f),
    phone(ph), status(st), countUnreadMessages(um.toInt())
{
    setMaximumSize(200, 60);

    QLabel* profileIcon = new QLabel(this);
    profileIcon->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(64, 64));

    QLabel* profileName = new QLabel(this);
    profileName->setText(login);
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

int FriendWidget::getCountUnreadMessages() const
{
    return countUnreadMessages;
}
