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
#include <QDate>
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

        static QDate lastDate;
        bool lastDateBool = false;
        bool first = false;

        while(str != "")
        {
            int p = str.indexOf("/!/");
            QString s = str.left(p);

            int pos = s.indexOf("!");

            QString idFriend = s.left(pos);

            s.remove(0, pos+1);

            pos = s.indexOf("!");

            QString idMessage = s.left(pos);

            s.remove(0, pos+1);
            pos = s.indexOf("!");

            QString date = s.left(pos);

            s.remove(0, pos+1);
            pos = s.indexOf("!");
            QString time = s.left(pos);
            s.remove(0, pos+1);
            pos = s.indexOf("!");
            QString idChat = s.left(pos);
            QString message = s.mid(pos+1);

            pos = message.indexOf("!");
            QString status = message.mid(pos+1);

            message = message.left(pos);

            if(numberBlockMessage != "1"
                    && lastDate == QDate::fromString(date,"dd MMMM yyyy") && lastDateBool == false)
            {
                // удалаем streach
                if(numberBlockMessage == "2")
                {
                    messageVBox->takeAt(0)->widget()->deleteLater();
                }
                messageVBox->takeAt(0)->widget()->deleteLater();
                lastDateBool = true;
            }

            if(lastDateBool == false)
            {

                lastDate = QDate::fromString(date,"dd MMMM yyyy");
                lastDateBool = true;
            }

            if(idChat == id && idFriend == friendInf->id)
            {
                Message* m = new Message(message, time, Message::right);
                if(numberBlockMessage == "1")
                {
                    if(messageVBox->count() == 0 || Date != QDate::fromString(date,"dd MMMM yyyy"))
                    {
                        messageVBox->addWidget(new QLabel(date));
                        Date = QDate::fromString(date,"dd MMMM yyyy");
                    }
                    messageWidgets.append(m);
                    messageVBox->addWidget(m);
                }
                else
                {
                    if(Date != QDate::fromString(date,"dd MMMM yyyy"))
                    {
                        if(first == true)
                        {
                            messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                            Date = QDate::fromString(date,"dd MMMM yyyy");
                            scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                                      + 13);
                            first = false;
                        }
                        else if(first == false)
                        {
                            Date = QDate::fromString(date,"dd MMMM yyyy");
                            first = true;
                        }
                    }
                    messageWidgets.push_front(m);
                    messageVBox->insertWidget(0, m);
                    scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                              + m->sizeMessageL());
                }
            }
            else if(idChat == friendInf->id && idFriend == id)
            {
                Message* m = new Message(message, time, Message::left);
                if(numberBlockMessage == "1")
                {
                    if(messageVBox->count() == 0 || Date != QDate::fromString(date,"dd MMMM yyyy"))
                    {
                        messageVBox->addWidget(new QLabel(date));
                        Date = QDate::fromString(date,"dd MMMM yyyy");
                    }
                    messageWidgets.append(m);
                    messageVBox->addWidget(m);
                }
                else
                {
                    if(Date != QDate::fromString(date,"dd MMMM yyyy"))
                    {
                        if(first == true)
                        {
                            messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                            Date = QDate::fromString(date,"dd MMMM yyyy");
                            scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                                      + 13);
                            first = false;
                        }
                        else if(first == false)
                        {
                            Date = QDate::fromString(date,"dd MMMM yyyy");
                            first = true;
                        }
                    }
                    messageWidgets.push_front(m);
                    messageVBox->insertWidget(0, m);
                    scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                              + m->sizeMessageL());
                }
                if(status == "1")
                {
                    friendInf->readUnreadMessages();
                    SlotSendToServer("/readUnreadMessages/" + idMessage + "!"  + friendInf->id);
                    QThread::msleep(15);
                }
            }
            if(isScrolling == true)
            {
                QTimer::singleShot(100, this, [this](){
                    scrollarea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);});
            }

            str.remove(0, p+3);
            if(numberBlockMessage != "1" && str == "")
            {
                Date = QDate::fromString(date,"dd MMMM yyyy");
                messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                          + 13);
            }
        }
    }

    if(buffer.indexOf("/newMessage/") != -1)
    {
        buffer.remove(0, 12);

        int pos = buffer.indexOf("!");

        QString idSender = buffer.left(pos);

        buffer.remove(0, pos+1);
        pos = buffer.indexOf("!");

        QString idMessage = buffer.left(pos);

        buffer.remove(0, pos+1);
        pos = buffer.indexOf("!");

        QString date = buffer.left(pos);
        buffer.remove(0, pos+1);
        pos = buffer.indexOf("!");
        QString time = buffer.left(pos);
        buffer.remove(0, pos+1);
        pos = buffer.indexOf("!");
        QString idChat = buffer.left(pos);
        QString message = buffer.mid(pos+1);

        pos = message.indexOf("!");
        QString status = message.mid(pos+1);

        message = message.left(pos);

        if(friendInf != nullptr && (friendInf->id == idSender || id == idSender))
        {
            if(Date != QDate::fromString(date,"dd MMMM yyyy"))
            {
                Date = QDate::fromString(date,"dd MMMM yyyy");
                messageVBox->addWidget(new QLabel(Date.toString("dd MMMM yyyy")));
            }
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
            if(status == "1")
            {
                SlotSendToServer("/readUnreadMessages/" + idMessage + "!"  + friendInf->id);
            }
            if(isScrolling == true)
            {
                QTimer::singleShot(100, this, [this](){
                    scrollarea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);});
            }
        }
        else
        {
            for(int i = 0; i < friendWidgets.count(); i++)
            {
                if(friendWidgets[i]->id == idSender)
                {
                    friendWidgets[i]->newUnreadMessages();
                    break;
                }
            }
        }
        if(idSender != id)
        {
            player->play();
        }
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

void MainWindow::cleanLayout(QLayout* oL)
{
    QLayoutItem *poLI;
    QLayout *poL;
    QWidget *poW;

    while((poLI = oL->takeAt(0)))
    {
        if((poL = poLI->layout()))
        {
            cleanLayout(poL);
            delete poL;
        }
        else if((poW = poLI->widget()))
        {
            delete poW;
        }
    }
}

void MainWindow::clickedFriendWidget(FriendWidget *friendW)
{
    isScrolling = true;
    numberBlockMessage = "1";

    cleanLayout(rightVBox);

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
    connect(scrollarea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(requestNewMessage(int)));
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
    connect(lineMessage, SIGNAL(editingFinished()), this, SLOT(clickedSendMessageButton()));

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

    SlotSendToServer("/HistoryMessage/" + friendInf->id + "!" + numberBlockMessage);
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
    if(lineMessage->text() != "")
    {
        SlotSendToServer("/message/" + friendInf->id + "!" + lineMessage->text());
        lineMessage->clear();
    }
}

void MainWindow::requestNewMessage(int value)
{
    if(value == 0 && messageVBox->count() >= 20)
    {
        isScrolling = false;
        numberBlockMessage = QString::number(numberBlockMessage.toInt() + 1);
        SlotSendToServer("/HistoryMessage/" + friendInf->id + "!" + numberBlockMessage);
    }
}
