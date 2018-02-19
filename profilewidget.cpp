#include "profilewidget.h"
#include <QIcon>
#include <QMouseEvent>

ProfileWidget::ProfileWidget(QString n, QWidget *parent) : QWidget(parent), name(n)
{
    QLabel* profileIcon = new QLabel(this);
    profileIcon->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(64, 64));

    profileName = new QLabel(this);
    profileName->setText(name);
    profileName->move(70, 15);

    QLabel* profileStatus = new QLabel(this);
    profileStatus->setText("online");
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
