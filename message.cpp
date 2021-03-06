#include "message.h"
#include <QBoxLayout>
#include <QDebug>

Message::Message(QString message, QString time, side s, QWidget *parent): QWidget(parent)
{
    QLabel* timeL = new QLabel(time);

    timeL->setStyleSheet("QLabel{color:#556c83;}");

    messageL = new QLabel(message);

    messageL->setAlignment(Qt::AlignCenter);

    messageL->setWordWrap(true);

    messageL->setStyleSheet("QLabel{ color:#fff; background:#2e3e4e; border-radius:5px;"
                            "padding-left: 3px; padding-right: 3px;"
                            "padding-bottom: 3px; padding-top: 3px;}");

    mainHBox = new QHBoxLayout(this);

    mainHBox->setMargin(5);

    if(s == side::left)
    {
        mainHBox->addWidget(messageL);
        mainHBox->addWidget(timeL);
        mainHBox->addStretch();
    }
    else
    {
        messageL->setAlignment(Qt::AlignRight);
        timeL->setAlignment(Qt::AlignRight);
        mainHBox->addStretch();
        mainHBox->addWidget(timeL);
        mainHBox->addWidget(messageL);
    }
    mainHBox->sizeHint().height();
}

int Message::sizeMessageL()
{
    return mainHBox->sizeHint().height();
}

QString Message::getMessage() const
{
    return messageL->text();
}

void Message::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    clicked();
}
