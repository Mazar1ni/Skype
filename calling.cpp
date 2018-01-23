#include "calling.h"
#include <QPushButton>
#include <QLabel>

calling::calling(QString str, MainWindow* parentW) : parent(parentW)
{

    setMinimumSize(500, 70);

    int pos = str.indexOf(" ");

    pass = str.mid(pos+1);

    nameRoom = str.left(pos);

    pos = str.left(pos).indexOf("-");

    QString name = str.left(pos);

    QLabel* passRoomLabel = new QLabel(this);
    passRoomLabel->setText(name);
    passRoomLabel->move(15, 15);

    QPushButton* callButton = new QPushButton(this);
    callButton->setIcon(QIcon(":/Icons/call_icon.png"));
    callButton->move(150, 15);
    callButton->resize(32, 32);

    connect(callButton, SIGNAL(clicked(bool)), this, SLOT(upCalling()));

    QPushButton* noCallButton = new QPushButton(this);
    noCallButton->setIcon(QIcon(":/Icons/call_end_icon.png"));
    noCallButton->move(180, 15);
    noCallButton->resize(32, 32);

    connect(noCallButton, SIGNAL(clicked(bool)), this, SLOT(noUpCalling()));
}

void calling::upCalling()
{
    parent->upCalling(nameRoom, pass);
    deleteLater();
}

void calling::noUpCalling()
{
    parent->noUpCalling(nameRoom, pass);
    deleteLater();
}
