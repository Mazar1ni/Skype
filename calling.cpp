#include "calling.h"
#include <QPushButton>
#include <QLabel>

calling::calling(QString name, QString passRoom, MainWindow* parentW) : parent(parentW), nameRoom(name), pass(passRoom)
{
    setWindowFlags(Qt::WindowContextHelpButtonHint);

    setMinimumSize(500, 70);

    int pos = name.indexOf("-");

    QString nameFriend = name.left(pos);

    QLabel* nameRoomLabel = new QLabel(this);
    nameRoomLabel->setText(nameFriend);
    nameRoomLabel->move(15, 15);

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

    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(player);

    player->setPlaylist(playlist);
    playlist->addMedia(QUrl("qrc:/Sound/IncomingCall.wav"));
    playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);

    player->play();

    callTimer = new QTimer;
    connect(callTimer, &QTimer::timeout, [this](){
        noUpCalling();
    });
    callTimer->start(30000);
}

void calling::upCalling()
{
    parent->upCalling(nameRoom, pass);
    callTimer->stop();
    callTimer->deleteLater();

    player->stop();
    player->deleteLater();

    deleteLater();
}

void calling::noUpCalling()
{
    parent->noUpCalling(nameRoom, pass);
    callTimer->stop();
    callTimer->deleteLater();

    player->stop();
    player->deleteLater();

    deleteLater();
}
