#include "mainwindow.h"
#include "friendwidget.h"
#include "calling.h"
#include "sox.h"
#include "message.h"
#include "webcam.h"
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
#include <QBuffer>

MainWindow::MainWindow(QTcpSocket *Sock, QString str, Sox *Sox, WebCam *wb, QWidget *parent)
    : QWidget(parent), Socket(Sock), sox(Sox), webCam(wb)
{
    video.clear();
    // оформление окна
    resize(900, 600);
    setMinimumSize(800, 450);

    QStringList list = str.split("!");

    id = list[0];
    login = list[1];
    email = list[2];
    name = list[3];
    phone = list[4];

    QWidget* profileWidget = new QWidget;
    profileWidget->setMaximumSize(200, 110);

    QLabel* profileIcon = new QLabel(profileWidget);
    profileIcon->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(64, 64));

    QLabel* profileName = new QLabel(profileWidget);
    profileName->setText(name);
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
    QStringList list = str.split("!");

    /*
    list[0] - id
    list[1] - login
    list[2] - email
    list[3] - name
    list[4] - Phone
    list[5] - Status
    list[6] - Under Messages
    */

    if(list[5] == "0")
    {
        list[5] = "offline";
    }
    else
    {
        list[5] = "online";
    }

    FriendWidget* friendWidget = new FriendWidget(list[0], list[1],
            list[2], list[3], list[4], list[5], list[6]);

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
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            QMessageBox::critical(NULL,QObject::tr("Error"),
                                  "Unable to start a new call because another call has already been initiated");
            return;
        }
    }
    int pos = name.indexOf("-");

    QString nameFriend = name.left(pos);

    if(friendInf == nullptr || nameFriend != friendInf->getLogin())
    {
        for(int i = 0; i < friendWidgets.count(); i++)
        {
            if(nameFriend == friendWidgets[i]->getLogin())
            {
                clickedFriendWidget(friendWidgets[i]);
                QThread::msleep(30);
                break;
            }
        }
    }

    SlotSendToServer("/30/" + name + ":" + pass);
    isOpenedRoom = true;
    startRecord();
    DeviceOutput = AudioOutput->start();

    friendInf->setCallStatus(true);

    createCallWidget();

    SlotSendToServer("/beginnigCall/" + friendInf->id);

}

void MainWindow::endCall()
{
    QString idFr;

    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            idFr = friendWidgets.at(i)->id;
            SlotSendToServer("/message/" + friendWidgets.at(i)->id + "!" + "/endingCall/"  +
                             friendWidgets.at(i)->getTimeCall().toString("hh:mm:ss"));
            friendWidgets.at(i)->setCallStatus(false);
            friendWidgets.at(i)->setVideoStatus(false);
            break;
        }
    }

    QThread::msleep(15);

    SlotSendToServer("/endingCall/" + idFr);

    if(friendInf->id == idFr)
    {
        cleanLayout(rightVBox->itemAt(0)->layout());
        createMainFriendWidget();
    }

    isOpenedRoom = false;
    isCreatedRoom = false;
    QMetaObject::invokeMethod(sox, "stopRecord", Qt::DirectConnection);
    QMetaObject::invokeMethod(webCam, "stopRecord", Qt::DirectConnection);

    QThread::msleep(30);

    SlotSendToServer("/endCall/");
}

void MainWindow::clickedMicroButton()
{
    if(isMicro == true)
    {
        dynamic_cast<QPushButton*>(QObject::sender())->setIcon(QIcon(":/Icons/micro_off_icon.png"));
        QMetaObject::invokeMethod(sox, "stopRecord", Qt::DirectConnection);
    }
    else
    {
        dynamic_cast<QPushButton*>(QObject::sender())->setIcon(QIcon(":/Icons/micro_on_icon.png"));
        startRecord();
    }

    isMicro ? isMicro = false : isMicro = true;
}

void MainWindow::clickedVideoButton()
{
    if(isVideo == true)
    {
        dynamic_cast<QPushButton*>(QObject::sender())->setIcon(QIcon(":/Icons/video_off_icon.png"));
        QMetaObject::invokeMethod(webCam, "stopRecord", Qt::DirectConnection);
        SlotSendToServer("/stopRecordVideo/" + id);
    }
    else
    {
        dynamic_cast<QPushButton*>(QObject::sender())->setIcon(QIcon(":/Icons/video_on_icon.png"));
        SlotSendToServer("/startRecordVideo/" + id);
        startRecordVideo();
    }
    isVideo ? isVideo = false : isVideo = true;
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
    else if(buffer.indexOf("/camera/") != -1)
    {
        video = buffer;
    }
    else if(buffer.indexOf("/end/") != -1)
    {
        video += buffer;
        video.remove(0, 8);
        video.remove(video.size() - 6, 5);
        imgWin(video);
        video.clear();
    }
    else if(buffer.indexOf("/startRecordVideo/") != -1)
    {
        buffer.remove(0, 18);
        // P.S. buffer - это id отправителя
        turnVideoBroadcast(buffer.toInt(), true);
    }
    else if(buffer.indexOf("/stopRecordVideo/") != -1)
    {
        buffer.remove(0, 17);
        // P.S. buffer - это id отправителя
        turnVideoBroadcast(buffer.toInt(), false);
    }
    else if(str.indexOf("/9/") != -1)
    {
        startRecord();
    }
    else if(str.indexOf("/5/") != -1)
    {
        str.remove(0, 3);

        gettingFriends(str);
    }
    else if(str.indexOf("/getMessages/") != -1)
    {
        str.remove(0, 13);

        static QDate lastDate;
        bool lastDateBool = false;
        bool first = false;

        QStringList list = str.split("/!/");

        for(int i = 0; i < list.count() - 1; i++)
        {
            QStringList listMessage = list[i].split("!");

            QString idFriend = listMessage[0];
            QString idMessage = listMessage[1];
            QString date = listMessage[2];
            QString time = listMessage[3];
            QString idChat = listMessage[4];
            QString message = listMessage[5];
            QString status = listMessage[6];

            if(numberBlockMessage != "1"
                    && lastDate == QDate::fromString(date,"dd MMMM yyyy") && lastDateBool == false)
            {
                // удаляем streach
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

            if(message == "/beginningCall/")
            {
                if(idChat == id)
                {
                    message = name + " " + QString::fromLocal8Bit("начал разговор");
                }
                else
                {
                    message = friendInf->getName() + " " + QString::fromLocal8Bit("начал разговор");
                }
            }

            if(message.indexOf("/endingCall/") != -1)
            {
                message.remove(0, 12);

                message = QString::fromLocal8Bit("Звонок завершен. Продолжительность: ") + message;
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

            if(numberBlockMessage != "1" && i + 1 == list.count() - 1)
            {
                Date = QDate::fromString(date,"dd MMMM yyyy");
                messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                          + 13);
            }
        }
    }
    else if(str.indexOf("/newMessage/") != -1)
    {
        str.remove(0, 12);

        QStringList list = str.split("!");

        QString idSender = list[0];
        QString idMessage = list[1];
        QString date = list[2];
        QString time = list[3];
        QString idChat = list[4];
        QString message = list[5];
        QString status;

        if(list.count() == 7)
        {
            status = list[6];
        }

        if(message == "/beginningCall/")
        {
            if(idSender == id)
            {
                message = name + " " + QString::fromLocal8Bit("начал разговор");
            }
            else
            {
                message = friendInf->getName() + " " + QString::fromLocal8Bit("начал разговор");
            }
        }

        if(message.indexOf("/endingCall/") != -1)
        {
            message.remove(0, 12);

            message = QString::fromLocal8Bit("Звонок завершен. Продолжительность: ") + message;
        }

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
    else if(str.indexOf("/29/") != -1)
    {
        str.remove(0, 4);

        QStringList list = str.split("!");

        QString nameRoom = list[0];

        calling* callWidget = new calling(nameRoom,  list[1], this);
        callWidget->show();
    }
    else if(str.indexOf("/33/") != -1)
    {
        str.remove(0, 4);
        QStringList list = str.split(":");

        foreach (FriendWidget* friendW, friendWidgets)
        {
            if(friendW->id == list[0])
            {
                friendW->updateStatus(list[1]);
                if(friendInf != nullptr && friendInf == friendW)
                {
                    dynamic_cast<QLabel*>(mainScreenWithButtons->children().back())->setText(list[1]);
                }
            }
        }
    }
    else if(str.indexOf("/beginnigCall/") != -1)
    {
        str.remove(0, 14);

        if(friendInf->id == str)
        {
            createCallWidget();
            friendInf->setCallStatus(true);
        }
        else
        {
            for(int i = 0; i < friendWidgets.count(); i++)
            {
                if(friendWidgets[i]->id == str)
                {
                    friendWidgets[i]->setCallStatus(true);
                    clickedFriendWidget(friendWidgets[i]);
                }
            }
        }

        SlotSendToServer("/message/" + friendInf->id + "!" + "/beginningCall/");
    }
    else if(buffer.indexOf("/outoftheroom/") != -1)
    {  
        for(int i = 0; i < friendWidgets.count(); i++)
        {
            if(friendWidgets.at(i)->getCallStatus() == true)
            {
                friendWidgets.at(i)->setCallStatus(false);
                friendWidgets.at(i)->setVideoStatus(false);
                clickedFriendWidget(friendWidgets.at(i));
            }
        }
        isOpenedRoom = false;
        isCreatedRoom = false;
        QMetaObject::invokeMethod(sox, "stopRecord", Qt::DirectConnection);
        QMetaObject::invokeMethod(webCam, "stopRecord", Qt::DirectConnection);
    }
    else
    {
        video += buffer;
    }
}

void MainWindow::sendSound(QByteArray buff)
{
    Socket->write(buff);
}

void MainWindow::sendCamera(QByteArray buff)
{
    Socket->write(buff);
}

void MainWindow::imgWin(QByteArray buff)
{
    if(friendInf->getVideoStatus() == true)
    {
        QDataStream ds(&buff, QIODevice::ReadOnly);

        int format = - 1;
        int bytesCount = -1;
        int width = 0;
        int height = 0;

        ds >> format;
        ds >> bytesCount;
        ds >> width;
        ds >> height;
        uchar bytes[bytesCount];
        ds.readRawData((char*)bytes, bytesCount);

        QImage img(bytes, width, height, (QImage::Format)format);

        cam->resize(img.size());
        cam->setPixmap(QPixmap::fromImage(img));
        cam->update();
    }
}

void MainWindow::turnVideoBroadcast(int idSender, bool onOff)
{
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->id.toInt() == idSender)
        {
            friendWidgets.at(i)->setVideoStatus(onOff);
            break;
        }
    }

    if(friendInf->id.toInt() == idSender)
    {
        cleanLayout(camAndIconLayout);
        if(onOff == true)
        {
            cam = new QLabel;
            camAndIconLayout->addWidget(cam, Qt::AlignCenter);
        }
        else
        {
            iconFriend = new QLabel;
            iconFriend->setPixmap(friendInf->getProfileIcon().pixmap(64, 64));
            iconFriend->setAlignment(Qt::AlignCenter);
            camAndIconLayout->addWidget(iconFriend, Qt::AlignCenter);
        }
    }
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

void MainWindow::createCallWidget()
{
    // удаляем верхний layout, где информаия о друге и кнопки вызова
    if(rightVBox->count() > 0)
    {
        QLayoutItem *poLI;

        while((poLI = rightVBox->itemAt(0)->layout()->takeAt(0)))
        {
            delete poLI->widget();
        }
        delete rightVBox->takeAt(0);
    }

    QVBoxLayout* callVBox = new QVBoxLayout;

    QHBoxLayout* buttonsHBox = new QHBoxLayout;

    QWidget* buttons = new QWidget;
    buttons->setFixedHeight(48);

    buttonsHBox->addWidget(buttons, Qt::AlignCenter);

    QPushButton* microButton = new QPushButton(buttons);
    if(isMicro == true)
    {
        microButton->setIcon(QIcon(":/Icons/micro_on_icon.png"));
    }
    else
    {
        microButton->setIcon(QIcon(":/Icons/micro_off_icon.png"));
    }
    microButton->move(15, 15);
    microButton->resize(32, 32);

    connect(microButton, SIGNAL(clicked(bool)), this, SLOT(clickedMicroButton()));

    QPushButton* videoButton = new QPushButton(buttons);
    if(isVideo == true)
    {
        videoButton->setIcon(QIcon(":/Icons/video_on_icon.png"));
    }
    else
    {
        videoButton->setIcon(QIcon(":/Icons/video_off_icon.png"));
    }
    videoButton->move(62, 15);
    videoButton->resize(32, 32);

    connect(videoButton, SIGNAL(clicked(bool)), this, SLOT(clickedVideoButton()));

    QPushButton* callEndButton = new QPushButton(buttons);
    callEndButton->setIcon(QIcon(":/Icons/call_end_icon.png"));
    callEndButton->move(109, 15);
    callEndButton->resize(32, 32);

    connect(callEndButton, SIGNAL(clicked(bool)), this, SLOT(endCall()));

    camAndIconLayout = new QHBoxLayout;

    if(friendInf->getVideoStatus() == true)
    {
        cam = new QLabel;
        camAndIconLayout->addWidget(cam, Qt::AlignCenter);
    }
    else
    {
        iconFriend = new QLabel;
        iconFriend->setPixmap(friendInf->getProfileIcon().pixmap(64, 64));
        iconFriend->setAlignment(Qt::AlignCenter);
        camAndIconLayout->addWidget(iconFriend, Qt::AlignCenter);
    }



    QLabel* friendName = new QLabel(friendInf->getName());
    friendName->setFixedHeight(15);
    friendName->setAlignment(Qt::AlignCenter);

    QLabel* timeCall = new QLabel();
    timeCall->setFixedHeight(15);
    timeCall->setAlignment(Qt::AlignCenter);

    connect(friendInf, SIGNAL(updateTimeCall(QString)), timeCall, SLOT(setText(QString)));

    callVBox->addLayout(camAndIconLayout, Qt::AlignCenter);
    callVBox->addWidget(friendName);
    callVBox->addWidget(timeCall);
    callVBox->addLayout(buttonsHBox);

    rightVBox->insertLayout(0, callVBox);
}

void MainWindow::createMainFriendWidget()
{
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

    hBox->addWidget(mainScreenWithButtons);
    hBox->addWidget(buttons);

    rightVBox->insertLayout(0, hBox);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event); // чтоб не ругался на неиспользуемую переменную
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            endCall();
            break;
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

    if(friendInf->getCallStatus() == true)
    {
        createCallWidget();
    }
    else
    {
        createMainFriendWidget();
    }

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

    rightVBox->addWidget(scrollarea);
    rightVBox->addLayout(lineMessageBox);

    SlotSendToServer("/HistoryMessage/" + friendInf->id + "!" + numberBlockMessage);
}

void MainWindow::clickedCallButton()
{
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            QMessageBox::critical(NULL,QObject::tr("Error"),
                                  "Unable to start a new call because another call has already been initiated");
            return;
        }
    }
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
