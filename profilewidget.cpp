#include "profilewidget.h"
#include "filetransfer.h"
#include <QIcon>
#include <QPixmap>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>

ProfileWidget::ProfileWidget(QString n, QString iconN, QString i, QString identNumber, QWidget *parent)
    : QWidget(parent), name(n), iconName(iconN), id(i), identifiacationNumber(identNumber)
{
    profileIcon = new QLabel(this);
    profileIcon->setMinimumSize(64, 64);

    // �������� ����������� �� ������ � �����
    if(iconName == "standart_icon.png")
    {
        profileIcon->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(64, 64));
    }
    else
    {
        QDir().mkdir("IconFriends");
        // ���� ���� ������ ���, �� ��� ����������� � �������
        if(!QFile::exists("IconFriends/" + id + "!" + iconName))
        {
            FileTransfer* fileTransfer = new FileTransfer(id, identifiacationNumber,
                                                          "downloadMainIcon", id + "!" + iconName);
            fileTransfer->start();
            connect(fileTransfer, SIGNAL(updateIconFriend()), this, SLOT(updateIcon()));
        }
        else
        {
            profileIcon->setPixmap(QIcon("IconFriends/" + id + "!" + iconName).pixmap(64, 64));
        }
    }

    profileName = new QLabel(this);
    profileName->setText(name);
    profileName->move(70, 15);

    QLabel* profileStatus = new QLabel(this);
    profileStatus->setText(tr("online"));
    profileStatus->move(70, 35);
}

void ProfileWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        clicked();
    }
}

void ProfileWidget::updateName(QString newName)
{
    name = newName;
    profileName->setText(name);
}

void ProfileWidget::updateIconName(QString newName)
{
    iconName = newName;
}

void ProfileWidget::updateIcon()
{
    profileIcon->setPixmap(QIcon("IconFriends/" + id + "!" + iconName).pixmap(64, 64));
}
