#include "mainwindow.h"
#include "friendwidget.h"
#include "calling.h"
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QTabWidget>
#include <sstream>
#include <QFile>
#include <QtMultimedia/QAudioFormat>
#include <QHeaderView>
#include <QMediaPlayer>
#include <QScrollBar>
#include <QTimer>
#include "sox.h"
#include "message.h"

MainWindow::MainWindow(QTcpSocket *Sock, QString str, Sox *Sox, QWidget *parent)
    : QWidget(parent), Socket(Sock), sox(Sox)
{
    // оформление окна
    resize(900, 600);
    setMinimumSize(800, 450);

    std::istringstream ist(str.toStdString());
    std::string tmp;
    QStringList list;
    while (ist >> tmp)
    {
        list << QString::fromUtf8(tmp.c_str());;
    }

    id = list[0];
    login = list[1];
    email = list[2];
    fam = list[3];
    name = list[4];
    otch = list[5];
    phone = list[6];

    QWidget* profileWidget = new QWidget;
    profileWidget->setMaximumSize(200, 110);

    QLabel* profileIcon = new QLabel(profileWidget);
    profileIcon->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(64, 64));

    QLabel* profileName = new QLabel(profileWidget);
    profileName->setText(login);
    profileName->move(70, 15);

    QLabel* profileStatus = new QLabel(profileWidget);
    profileStatus->setText("online");
    profileStatus->move(70, 35);

    QLineEdit* search = new QLineEdit(profileWidget);
    search->resize(150, 20);
    search->move(10, 80);

    QWidget* friendsWidget = new QWidget;
    friendsWidget->setMaximumSize(200, this->size().height());
    friendsVBox = new QVBoxLayout;
    friendsVBox->setAlignment(Qt::AlignTop);
    friendsWidget->setLayout(friendsVBox);

    QWidget* recentWidget = new QWidget;
    recentWidget->setMaximumSize(200, this->size().height());

    QTabWidget* listFriendsAndRecent = new QTabWidget;
    listFriendsAndRecent->addTab(friendsWidget, "Friends");
    listFriendsAndRecent->addTab(recentWidget, "Recent");
    listFriendsAndRecent->setStyleSheet(QString("QTabBar::tab {width: 100px; height: 35px;}"));
    listFriendsAndRecent->setFixedWidth(200);

    QVBoxLayout* leftVBox = new QVBoxLayout;
    leftVBox->addWidget(profileWidget, Qt::AlignTop);
    leftVBox->addWidget(listFriendsAndRecent, Qt::AlignTop);
    leftVBox->addStretch();

    createSettingRoomWidget();

    rightVBox = new QVBoxLayout;
    rightVBox->addWidget(settingRoom);

    QHBoxLayout* mainHBox = new QHBoxLayout;

    mainHBox->addLayout(leftVBox, Qt::AlignLeft);
    mainHBox->addLayout(rightVBox, Qt::AlignLeft);

    this->setLayout(mainHBox);

    connect(Socket, SIGNAL(readyRead()), SLOT(SlotReadyRead()));

    Format.setSampleRate(4000);
    Format.setChannelCount(1);
    Format.setSampleSize(16);
    Format.setCodec("audio/pcm");
    Format.setByteOrder(QAudioFormat::LittleEndian);
    Format.setSampleType(QAudioFormat::UnSignedInt);

    AudioOutput = new QAudioOutput(Format, this);

    player = new QMediaPlayer();
    player->setMedia(QUrl("qrc:/Sound/newMessage.wav"));

    // получение от сервера друзей пользователя
    SlotSendToServer("/4/");
}

void MainWindow::SlotSendToServer(QString str)
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << str;

    Socket->write(arrBlock);
    Socket->flush();
}

void MainWindow::gettingFriends(QString str)
{
    std::istringstream ist(str.toStdString());
    std::string tmp;
    QStringList list;
    while (ist >> tmp)
    {
        list << QString::fromUtf8(tmp.c_str());;
    }

    /*
    list[0] - id
    list[1] - login
    list[2] - email
    list[3] - F
    list[4] - I
    list[5] - O
    list[6] - Phone
    list[7] - Status
    list[8] - Under Messages
    */

    if(list[7] == "0")
    {
        list[7] = "offline";
    }
    else
    {
        list[7] = "online";
    }

    FriendWidget* friendWidget = new FriendWidget(list[0], list[1],
            list[2] + " " + list[3] + " "  + list[4], list[5], list[6], list[7], list[8]);

    friendsVBox->addWidget(friendWidget, Qt::AlignTop);

    friendWidgets.append(friendWidget);

    connect(friendWidget, SIGNAL(clicked(FriendWidget*)), this, SLOT(clickedFriendWidget(FriendWidget*)));

}

void MainWindow::createSettingRoomWidget()
{
    settingRoom = new QWidget;

    QLabel* nameRoomLabel = new QLabel(settingRoom);
    nameRoomLabel->setText("name:");
    nameRoomLabel->move(10, 0);

    nameRoom = new QLineEdit(settingRoom);
    nameRoom->resize(250, 20);
    nameRoom->move(10, 20);

    QLabel* passRoomLabel = new QLabel(settingRoom);
    passRoomLabel->setText("password:");
    passRoomLabel->move(10, 50);

    passRoom = new QLineEdit(settingRoom);
    passRoom->resize(250, 20);
    passRoom->move(10, 70);

    QPushButton* createRoom = new QPushButton(settingRoom);
    createRoom->resize(100, 30);
    createRoom->move(10, 100);
    createRoom->setText("Create");

    connect(createRoom, SIGNAL(clicked(bool)), this, SLOT(CreateRoom()));

    QPushButton* openRoom = new QPushButton(settingRoom);
    openRoom->resize(100, 30);
    openRoom->move(160, 100);
    openRoom->setText("Open");

    connect(openRoom, SIGNAL(clicked(bool)), this, SLOT(OpenRoom()));
}

void MainWindow::upCalling(QString name, QString pass)
{
    SlotSendToServer("/30/" + name + ":" + pass);
    isOpenedRoom = true;
}

void MainWindow::noUpCalling(QString name, QString pass)
{
    SlotSendToServer("/31/" + name + ":" + pass);
}

void MainWindow::CreateRoom()
{
    SlotSendToServer(nameRoom->text() + ":" + passRoom->text() + "/2/");

    isCreatedRoom = true;
    startRecord();
    DeviceOutput = AudioOutput->start();
}

void MainWindow::OpenRoom()
{
    SlotSendToServer(nameRoom->text() + ":" + passRoom->text() + "/3/");

    isOpenedRoom = true;
}

void MainWindow::SlotReadyRead()
{
    QByteArray buffer;
    buffer = Socket->readAll();

    QDataStream in(buffer);

    QString str;
    in >> str;

    if(buffer.indexOf("/18/") != -1)
    {
        buffer.remove(0, 4);
        DeviceOutput->write(buffer);
    }

    if(str.indexOf("/9/") != -1)
    {
        startRecord();
    }

    if(str.indexOf("/5/") != -1)
    {
        str.remove(0, 3);

        gettingFriends(str);
    }

    if(str.indexOf("/getMessages/") != -1)
    {
        str.remove(0, 13);

        int pos = str.indexOf("!");

        QString time = str.left(pos);
        str.remove(0, pos+1);
        pos = str.indexOf("!");
        QString idChat = str.left(pos);
        QString message = str.mid(pos+1);

        pos = message.indexOf("/lastMessage/");

        if(pos != -1)
        {
            if(friendInf->getCountUnreadMessages() != 0)
            {
                friendInf->clearUnreadMessages();
                SlotSendToServer("/readAllMessages/" + friendInf->id);
            }
            message.remove(pos, message.count());
        }

        if(idChat == id)
        {
            Message* m = new Message(message, time, Message::right);
            messageWidgets.append(m);
            messageVBox->addWidget(m);
        }
        else
        {
            Message* m = new Message(message, time, Message::left);
            messageWidgets.append(m);
            messageVBox->addWidget(m);
        }
        QTimer::singleShot(100, this, [this](){
            scrollarea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);});

    }

    if(buffer.indexOf("/newMessage/") != -1)
    {
        buffer.remove(0, 12);

        int pos = buffer.indexOf("!");

        QString idSender = buffer.left(pos);

        buffer.remove(0, pos+1);
        pos = buffer.indexOf("!");

        QString time = buffer.left(pos);
        buffer.remove(0, pos+1);
        pos = buffer.indexOf("!");
        QString idChat = buffer.left(pos);
        QString message = buffer.mid(pos+1);

        if(friendInf != nullptr && (friendInf->id == idSender || id == idSender))
        {
            if(idChat != id)
            {
                Message* m = new Message(message, time, Message::right);
                messageWidgets.append(m);
                messageVBox->addWidget(m);
            }
            else
            {
                Message* m = new Message(message, time, Message::left);
                messageWidgets.append(m);
                messageVBox->addWidget(m);
            }
            QTimer::singleShot(100, this, [this](){
                scrollarea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);});
        }
        else
        {
            for(int i = 0; i < friendWidgets.count(); i++)
            {
                if(friendWidgets[i]->id == idSender)
                {
                    friendWidgets[i]->updateUnreadMessages();
                    break;
                }
            }
        }
        player->play();
    }

    if(buffer.indexOf("/29/") != -1)
    {
        buffer.remove(0, 4);

        calling* callWidget = new calling(buffer, this);
        callWidget->show();
    }

    if(buffer.indexOf("/33/") != -1)
    {
        buffer.remove(0, 4);
        int pos = buffer.indexOf(":");

        foreach (FriendWidget* friendW, friendWidgets) {
            if(friendW->id == buffer.left(pos))
            {
                friendW->updateStatus(buffer.mid(pos + 1));
            }
        }
    }

    if(buffer.indexOf("/outoftheroom/") != -1)
    {
        isCreatedRoom = false;
        stopRecord();
        DeviceOutput = nullptr;
    }

}

void MainWindow::sendSound(QByteArray buff)
{
    Socket->write(buff);
}

void MainWindow::SlotSendAudioToServer()
{
    //emit(removeNoise());

    /*QMetaObject::invokeMethod(sox, "removeNoise", Qt::AutoConnection,
                              Q_ARG(MainWindow*, this),
                              Q_ARG(QByteArray, arrBlock))*/;

    //Socket->write(arrBlock);
}

void MainWindow::clickedFriendWidget(FriendWidget *friendW)
{
    if(friendInf == nullptr)
    {
        delete rightVBox->itemAt(0);
        delete settingRoom;
    }
    else
    {
        return;
    }

    messageWidgets.clear();

    friendInf = friendW;

    QHBoxLayout* hBox = new QHBoxLayout;

    mainScreenWithButtons = new QWidget;
    mainScreenWithButtons->setMinimumSize(200, 110);

    QLabel* profileIconRightWidget = new QLabel(mainScreenWithButtons);
    profileIconRightWidget->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(64, 64));

    QLabel* profileNameRightWidget = new QLabel(mainScreenWithButtons);
    profileNameRightWidget->setText(friendInf->login);
    profileNameRightWidget->move(70, 15);

    QLabel* profileStatusRightWidget = new QLabel(mainScreenWithButtons);
    profileStatusRightWidget->setText(friendInf->status);
    profileStatusRightWidget->move(70, 35);

    QWidget* buttons = new QWidget;

    QPushButton* callButton = new QPushButton(buttons);
    callButton->setIcon(QIcon(":/Icons/call_icon.png"));
    callButton->move(15, 15);
    callButton->resize(32, 32);

    connect(callButton, SIGNAL(clicked(bool)), this, SLOT(clickedCallButton()));

    QPushButton* videoButton = new QPushButton(buttons);
    videoButton->setIcon(QIcon(":/Icons/video_on_icon.png"));
    videoButton->move(62, 15);
    videoButton->resize(32, 32);

    scrollarea = new QScrollArea();
    //scrollarea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollarea->setWidgetResizable(true);
    scrollarea->setFrameStyle(QFrame::NoFrame);

    QWidget *messageWidget = new QWidget();
    scrollarea->setWidget(messageWidget);

    messageVBox = new QVBoxLayout();
    messageVBox->addStretch();
    messageWidget->setLayout(messageVBox);

    QHBoxLayout* lineMessageBox = new QHBoxLayout;

    lineMessage = new QLineEdit;
    lineMessage->setMaximumHeight(40);
    lineMessage->setMinimumHeight(40);

    QPushButton* sendMessage = new QPushButton;
    sendMessage->setIcon(QIcon(":/Icons/chat_icon.png"));
    sendMessage->setIconSize(QSize(30, 30));
    sendMessage->setMinimumSize(80, 40);

    connect(sendMessage, SIGNAL(clicked(bool)), this, SLOT(clickedSendMessageButton()));

    lineMessageBox->addWidget(lineMessage);
    lineMessageBox->addWidget(sendMessage);

    hBox->addWidget(mainScreenWithButtons);
    hBox->addWidget(buttons);

    rightVBox->addLayout(hBox);
    rightVBox->addWidget(scrollarea);
    rightVBox->addLayout(lineMessageBox);

    SlotSendToServer("/HistoryMessage/" + friendInf->id);
}

void MainWindow::clickedCallButton()
{
    if(friendInf->status == "online")
    {
        SlotSendToServer("/19/" + friendInf->id);

        isCreatedRoom = true;

        DeviceOutput = AudioOutput->start();
        startRecord();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Friend not online");
        msgBox.exec();
    }
}

void MainWindow::clickedSendMessageButton()
{
    SlotSendToServer("/message/" + friendInf->id + "!" + lineMessage->text());
    QThread::msleep(15);
    SlotSendToServer("/UnreadMessage/" + friendInf->id);
    lineMessage->clear();
}
