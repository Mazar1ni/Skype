#include "mainwindow.h"
#include "friendwidget.h"
#include "calling.h"
#include "audio.h"
#include "message.h"
#include "webcam.h"
#include "profilewidget.h"
#include "filetransfer.h"
#include "chatwidget.h"
#include "settingswidget.h"
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QTabWidget>
#include <sstream>
#include <QFile>
#include <QHeaderView>
#include <QMediaPlayer>
#include <QScrollBar>
#include <QTimer>
#include <QDate>
#include <QBuffer>
#include <QMenu>

MainWindow::MainWindow(QTcpSocket *Sock, QString str, Audio *a, WebCam *wb, QWidget *parent)
    : QWidget(parent), Socket(Sock), audio(a), webCam(wb)
{
    video.clear();
    // оформление окна
    resize(900, 600);
    setMinimumSize(800, 450);
    this->setWindowIcon(QIcon(":/Icons/skype_icon.ico"));

    Date.setDate(1900, 1, 1);

    // создание иконки в трее
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Icons/skype_icon.ico"));
    trayIcon->setToolTip("Skype");

    // меню для иконки в трее
    QMenu * menu = new QMenu(this);
    QAction * quitAction = new QAction(tr("Quit"), this);

    connect(quitAction, &QAction::triggered, [this](){
        for(int i = 0; i < friendWidgets.count(); i++)
        {
            if(friendWidgets.at(i)->getCallStatus() == true)
            {
                endCall();
                break;
            }
        }
        isFinalClosing = true;
        close();
    });

    menu->addAction(quitAction);

    trayIcon->setContextMenu(menu);
    trayIcon->show();

    // по нажатию на иконку в трее показывать/скрывать главное окно
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    // информация о пользователе полученная от сервера
    QStringList list = str.split("!");

    id = list[0];
    login = list[1];
    email = list[2];
    name = list[3];
    phone = list[4];
    identificationNumber = list[5];
    iconName = list[6];

    // создание виджета профиля
    profileWidget = new ProfileWidget(name, iconName, id, identificationNumber);
    profileWidget->setMaximumSize(200, 90);

    // по нажатию на виджет профиля открывать настройки пользователя
    connect(profileWidget, SIGNAL(clicked()), this, SLOT(clickedProfileWidget()));

    // поиск по друзьям и потенциальным друзьям
    QLineEdit* search = new QLineEdit;
    search->setPlaceholderText(tr("search..."));
    search->resize(150, 20);
    search->setMaximumWidth(200);
    // понажатию enter в строке поиска, отправлять на сервер запрос поиска
    connect(search, SIGNAL(textChanged(QString)), this, SLOT(search(QString)));

    // создание столбца друзья
    QScrollArea* scrollareaFriends = new QScrollArea();
    scrollareaFriends->setWidgetResizable(true);
    scrollareaFriends->setFrameStyle(QFrame::NoFrame);

    QWidget* friendsWidget = new QWidget;
    scrollareaFriends->setWidget(friendsWidget);

    friendsVBox = new QVBoxLayout;
    friendsVBox->setAlignment(Qt::AlignTop);
    friendsWidget->setLayout(friendsVBox);

    // создание столбца последние
    QScrollArea* scrollareaRecent = new QScrollArea();
    scrollareaRecent->setWidgetResizable(true);
    scrollareaRecent->setFrameStyle(QFrame::NoFrame);

    QWidget* recentWidget = new QWidget;
    scrollareaRecent->setWidget(recentWidget);

    recentVBox = new QVBoxLayout;
    recentVBox->setAlignment(Qt::AlignTop);
    recentWidget->setLayout(recentVBox);

    QTabWidget* listFriendsAndRecent = new QTabWidget;
    listFriendsAndRecent->addTab(scrollareaFriends, tr("Friends"));
    listFriendsAndRecent->addTab(scrollareaRecent, tr("Recent"));
    listFriendsAndRecent->setStyleSheet(QString("QTabBar::tab {width: 100px; height: 35px;}"));
    listFriendsAndRecent->setFixedWidth(200);

    QVBoxLayout* leftVBox = new QVBoxLayout;
    leftVBox->addWidget(profileWidget, Qt::AlignTop);
    leftVBox->addWidget(search, Qt::AlignTop);
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

    // установка мелодии для уведомления
    player = new QMediaPlayer();
    player->setMedia(QUrl("qrc:/Sound/newMessage.wav"));

    // установка мелодии для звонка
    callPlayer = new QMediaPlayer();
    callPlayer->setMedia(QUrl("qrc:/Sound/phoneRings.wav"));

    // получение от сервера приглашений в друзья
    SlotSendToServer("/inviteFr/");

    // получение от сервера друзей пользователя
    QTimer::singleShot(1000, [this](){
        SlotSendToServer("/4/");
    });

    // получение от сервера последние звонки пользователя
    QTimer::singleShot(2000, [this](){
        SlotSendToServer("/recent/");
    });

    // отправка сигнала подключения к аудио серверу
    QTimer::singleShot(2000, [this](){
        connectSoundServer(id, identificationNumber);
    });
}

// слот для отправки сообщений серверу
void MainWindow::SlotSendToServer(QString str)
{
    //qDebug() << "send:  " << str;

    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << str;

    Socket->write(arrBlock);
    Socket->flush();
}

// добавление друзей/потенциальных друзей/последние/запросы в друзья на layout
void MainWindow::gettingFriends(QString str, QString mode)
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
    list[7] - Icon Name
    */

    // кастомизация статуса пользователя
    if(list[5] == "0")
    {
        list[5] = tr("offline");
    }
    else
    {
        list[5] = tr("online");
    }

    bool isFriend;

    mode == "potentialFriends" || mode == "inviteToFriend" ? isFriend = false : isFriend = true;

    FriendWidget* friendWidget = new FriendWidget(list[0], list[1],
            list[2], list[3], list[4], list[5], list[6], list[7], id, identificationNumber, isFriend, this);

    // проверка по mode к какому типу относиться переданный пользователь
    if(mode == "friend")
    {
        friendsVBox->addWidget(friendWidget, Qt::AlignTop);

        friendWidgets.append(friendWidget);
    }
    else if(mode == "recent")
    {
        recentVBox->addWidget(friendWidget, Qt::AlignTop);
        recentWidgets.append(friendWidget);
    }
    else if(mode == "newRecent")
    {
        recentVBox->insertWidget(0, friendWidget, Qt::AlignTop);
        recentWidgets.append(friendWidget);
    }
    else if(mode == "potentialFriends")
    {
        QString s = list[8];
        friendWidget->setIsEnableInviteToFriend(s.toInt());
        friendsVBox->addWidget(friendWidget, Qt::AlignTop);
        potentialFriendsWidgets.append(friendWidget);
    }
    else if(mode == "inviteToFriend")
    {
        friendWidget->setIsAcceptFriendInvitation("true");
        friendsVBox->insertWidget(0, friendWidget, Qt::AlignTop);
    }

    // по нажатию на виджет пользователя открывать чат с ним
    connect(friendWidget, SIGNAL(clicked(FriendWidget*)), this, SLOT(clickedFriendWidget(FriendWidget*)));
}

// добавление нового недавнего вызова на самый верх layout'а
void MainWindow::addRecent(QString id)
{
    for(int i = 0; i < recentWidgets.size(); i++)
    {
        if(recentWidgets.at(i)->getId() == id)
        {
            recentVBox->removeWidget(recentWidgets.at(i));
            recentVBox->insertWidget(0, recentWidgets.at(i));
            return;
        }
    }

    // отправка на сервер нового последнего вызова
    SlotSendToServer("/newRecent/" + id);
}

// создание виджета создания/открытия комнаты
// !!!---не работает---!!!
void MainWindow::createSettingRoomWidget()
{
    settingRoom = new QWidget;

    QLabel* nameRoomLabel = new QLabel(settingRoom);
    nameRoomLabel->setText(tr("name:"));
    nameRoomLabel->move(10, 0);

    nameRoom = new QLineEdit(settingRoom);
    nameRoom->resize(250, 20);
    nameRoom->move(10, 20);

    QLabel* passRoomLabel = new QLabel(settingRoom);
    passRoomLabel->setText(tr("password:"));
    passRoomLabel->move(10, 50);

    passRoom = new QLineEdit(settingRoom);
    passRoom->resize(250, 20);
    passRoom->move(10, 70);

    QPushButton* createRoom = new QPushButton(settingRoom);
    createRoom->resize(100, 30);
    createRoom->move(10, 100);
    createRoom->setText(tr("Create"));

    //connect(createRoom, SIGNAL(clicked(bool)), this, SLOT(CreateRoom()));

    QPushButton* openRoom = new QPushButton(settingRoom);
    openRoom->resize(100, 30);
    openRoom->move(160, 100);
    openRoom->setText(tr("Open"));

    //connect(openRoom, SIGNAL(clicked(bool)), this, SLOT(OpenRoom()));
}

// принятие вызова
void MainWindow::upCalling(QString name, QString pass)
{
    // если соединения с аудио сервером нет, то нельзя принять звонок
    if(isConnectedAudio == false)
    {
        return;
    }
    // проверка есть ли на данный момент существующие разговоры с другими пользователями
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            QMessageBox::critical(NULL,tr("Error"),
                                  tr("Unable to start a new call because another call has already been initiated"));
            return;
        }
    }
    int pos = name.indexOf("-");

    QString nameFriend = name.left(pos);

    // находим в списке друзей друга, который звонит и открываем с ним чат
    if(friendInf == nullptr || nameFriend != friendInf->getLogin())
    {
        for(int i = 0; i < friendWidgets.count(); i++)
        {
            if(nameFriend == friendWidgets[i]->getLogin())
            {
                clickedFriendWidget(friendWidgets[i]);
                break;
            }
        }
    }

    QThread::msleep(30);

    // отправка серверу открытия комнаты
    SlotSendToServer("/30/" + name + ":" + pass);
    isOpenedRoom = true;
    startRecord();

    friendInf->setCallStatus(true);

    // изменение состояния звонка
    for(int i = 0; i < recentWidgets.count(); i++)
    {
        if(recentWidgets[i]->getId() == friendInf->getId())
        {
            recentWidgets[i]->setCallStatus(true);
        }
    }

    createCallWidget();

    QThread::msleep(30);

    // отправка серверу согласие начать звонок
    SlotSendToServer("/beginnigCall/" + friendInf->getId());
}

// конец звонка
void MainWindow::endCall()
{
    QString idFr;

    // изменение состояния звонка
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            idFr = friendWidgets.at(i)->getId();
            // отправка этому другу сообщения об окончании звонка
            SlotSendToServer("/message/" + friendWidgets.at(i)->getId() + "!" + "/endingCall/"  +
                             friendWidgets.at(i)->getTimeCall().toString("hh:mm:ss"));
            friendWidgets.at(i)->setCallStatus(false);
            friendWidgets.at(i)->setVideoStatus(false);
            break;
        }
    }

    // изменение состояния звонка
    for(int i = 0; i < recentWidgets.count(); i++)
    {
        if(recentWidgets.at(i)->getCallStatus() == true)
        {
            recentWidgets.at(i)->setCallStatus(false);
            recentWidgets.at(i)->setVideoStatus(false);
            break;
        }
    }

    QThread::msleep(15);

    // отправка окончания звонка с этим другом
    SlotSendToServer("/endingCall/" + idFr);

    if(friendInf->getId() == idFr)
    {
        cleanLayout(rightVBox->itemAt(0)->layout());
        createMainFriendWidget();
    }

    isOpenedRoom = false;
    isCreatedRoom = false;
    QMetaObject::invokeMethod(audio, "stopRecord", Qt::DirectConnection);
    QMetaObject::invokeMethod(webCam, "stopRecord", Qt::DirectConnection);

    QThread::msleep(30);

    // отправка серверу окончания звонка и закрытие комнаты
    SlotSendToServer("/endCall/");
}

// включать/выключать микрофон по нажатию кнопки
void MainWindow::clickedMicroButton()
{
    if(isMicro == true)
    {
        dynamic_cast<QPushButton*>(QObject::sender())->setIcon(QIcon(":/Icons/micro_off_icon.png"));
        QMetaObject::invokeMethod(audio, "stopRecord", Qt::DirectConnection);
    }
    else
    {
        dynamic_cast<QPushButton*>(QObject::sender())->setIcon(QIcon(":/Icons/micro_on_icon.png"));
        startRecord();
    }

    isMicro ? isMicro = false : isMicro = true;
}

// включать/выключать веб-камеру по нажатию кнопки
void MainWindow::clickedVideoButton()
{
    if(webCam->getIsCamera() == false)
    {
        QMessageBox::critical(NULL,tr("Error"), tr("Webcam is not connected!"));
        return;
    }

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

// нажатие по профилю пользователя
void MainWindow::clickedProfileWidget()
{
    cleanLayout(rightVBox);

    QHBoxLayout* profileHBox = new QHBoxLayout;

    QVBoxLayout* profileLeftVBox = new QVBoxLayout;

    profileIcon = new QLabel;
    if(iconName == "standart_icon.png")
    {
        profileIcon->setPixmap(QIcon(":/Icons/standart_icon.png").pixmap(256, 256));
    }
    else
    {
        profileIcon->setPixmap(QIcon("IconFriends/" + id + "!" + iconName).pixmap(256, 256));
    }
    profileIcon->setMargin(30);

    QString qss = ("QPushButton{"
                              "font-weight: 700;"
                              "text-decoration: none;"
                              "padding: .5em 2em;"
                              "outline: none;"
                              "border: 2px solid;"
                              "border-radius: 1px;"
                            "} "
                            "QPushButton:!hover { background: rgb(255,255,255); }");

    QPushButton* changeIcon = new QPushButton;
    changeIcon->setText(tr("Change Icon"));
    changeIcon->setStyleSheet(qss);

    // по нажатию на кнопку изменение иконки пользователя создать объект и загрузить, выбранную пользователем иконку
    connect(changeIcon, &QPushButton::clicked, [this](){

        FileTransfer* fileTransfer = new FileTransfer(this->id, this->identificationNumber, "uploadMainIcon");
        fileTransfer->start();
        connect(fileTransfer, &FileTransfer::sendFriendsUpdateIcon, [this](){
            this->SlotSendToServer("/sendFriendsUpdateIcon/");
        });
    });

    QPushButton* settingsB = new QPushButton;
    settingsB->setText(tr("Settings"));
    settingsB->setStyleSheet(qss);

    // открывает настройки
    connect(settingsB, SIGNAL(clicked(bool)), this, SLOT(clickedSettings()));

    profileLeftVBox->addWidget(profileIcon);
    profileLeftVBox->addWidget(changeIcon);
    profileLeftVBox->addWidget(settingsB);
    profileLeftVBox->addStretch();

    // информация о пользователе

    QVBoxLayout* profileRightVBox = new QVBoxLayout;

    QHBoxLayout* profileNameHBox = new QHBoxLayout;

    QLabel* profileNameLabel = new QLabel(tr("Name:    "));

    QLineEdit* profileName = new QLineEdit;
    profileName->setText(name);

    profileNameHBox->addStretch();
    profileNameHBox->addWidget(profileNameLabel);
    profileNameHBox->addWidget(profileName);

    QHBoxLayout* profileEmailHBox = new QHBoxLayout;

    QLabel* profileEmailLabel = new QLabel(tr("Email:    "));

    QLineEdit* profileEmail = new QLineEdit;
    profileEmail->setText(email);

    profileEmailHBox->addStretch();
    profileEmailHBox->addWidget(profileEmailLabel);
    profileEmailHBox->addWidget(profileEmail);

    QHBoxLayout* profilePhoneHBox = new QHBoxLayout;

    QLabel* profilePhoneLabel = new QLabel(tr("Phone:    "));

    QLineEdit* profilePhone = new QLineEdit;
    profilePhone->setText(phone);

    profilePhoneHBox->addStretch();
    profilePhoneHBox->addWidget(profilePhoneLabel);
    profilePhoneHBox->addWidget(profilePhone);

    QHBoxLayout* profileOldPassHBox = new QHBoxLayout;

    QLabel* profileOldPassLabel = new QLabel(tr("enter old password:    "));

    QLineEdit* profileOldPass = new QLineEdit;
    profileOldPass->setEchoMode(QLineEdit::Password);
    profileOldPass->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);

    profileOldPassHBox->addStretch();
    profileOldPassHBox->addWidget(profileOldPassLabel);
    profileOldPassHBox->addWidget(profileOldPass);

    QHBoxLayout* profileNewPassHBox = new QHBoxLayout;

    QLabel* profileNewPassLabel = new QLabel(tr("enter new password:    "));

    QLineEdit* profileNewPass = new QLineEdit;
    profileNewPass->setEchoMode(QLineEdit::Password);
    profileNewPass->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);

    profileNewPassHBox->addStretch();
    profileNewPassHBox->addWidget(profileNewPassLabel);
    profileNewPassHBox->addWidget(profileNewPass);

    QHBoxLayout* profileNewPassAgainHBox = new QHBoxLayout;

    QLabel* profileNewPassAgainLabel = new QLabel(tr("enter new password again:    "));

    QLineEdit* profileNewPassAgain = new QLineEdit;
    profileNewPassAgain->setEchoMode(QLineEdit::Password);
    profileNewPassAgain->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);

    profileNewPassAgainHBox->addStretch();
    profileNewPassAgainHBox->addWidget(profileNewPassAgainLabel);
    profileNewPassAgainHBox->addWidget(profileNewPassAgain);

    QHBoxLayout* buttonsHBox = new QHBoxLayout;
    buttonsHBox->setMargin(15);

    QPushButton* saveChangesButton = new QPushButton;
    saveChangesButton->setFixedSize(84, 24);
    saveChangesButton->setIcon(QIcon(":/Icons/checkmark_circled_icon.png").pixmap(64, 64));
    saveChangesButton->setStyleSheet(qss);

    // проверка и отправка серверу данных для обновления информации о пользователе
    connect(saveChangesButton, &QPushButton::clicked, [this, profileName, profileEmail,
            profilePhone, profileOldPass, profileNewPass, profileNewPassAgain]()
    {
        if(this->name != profileName->text() && profileName->text() != "")
        {
            SlotSendToServer("/profileInfo/" + ("/name/" + profileName->text()));
            this->updateInfo("/name/" + profileName->text());
        }

        if(this->email != profileEmail->text() && profileEmail->text() != "")
        {
            SlotSendToServer("/profileInfo/" + ("/email/" + profileEmail->text()));
            this->updateInfo("/email/" + profileEmail->text());
        }

        if(this->phone != profilePhone->text() && profilePhone->text() != "")
        {
            SlotSendToServer("/profileInfo/" + ("/phone/" + profilePhone->text()));
            this->updateInfo("/phone/" + profilePhone->text());
        }

        if(profileOldPass->text() != "")
        {
            QString message;
            message = "/oldPass/" + profileOldPass->text();
            if(profileNewPass->text() != "")
            {
                if(profileNewPass->text() == profileOldPass->text())
                {
                    QMessageBox::critical(NULL,tr("Error"),
                                          tr("Old password and new password must not be the same!"));
                    return;
                }

                if(profileNewPass->text() != profileNewPassAgain->text())
                {
                    QMessageBox::critical(NULL,tr("Error"), tr("Passwords do not match!"));
                    return;
                }
                message += "/newPass/" + profileNewPass->text();
            }
            else
            {
                QMessageBox::critical(NULL,tr("Error"), tr("Fill in all the fields!"));
                return;
            }
            SlotSendToServer("/profileInfo/" + ("/changePass/" + message));
        }
    });

    QPushButton* canselChangesButton = new QPushButton;
    canselChangesButton->setFixedSize(84, 24);
    canselChangesButton->setIcon(QIcon(":/Icons/close_circled_icon.png").pixmap(64, 64));
    canselChangesButton->setStyleSheet(qss);

    // очистка и возврат к значениям поумолчанию полей
    connect(canselChangesButton, &QPushButton::clicked, [this, profileName, profileEmail,
            profilePhone, profileOldPass, profileNewPass, profileNewPassAgain](){
        profileName->setText(this->name);
        profileEmail->setText(this->email);
        profilePhone->setText(this->phone);

        profileNewPass->clear();
        profileOldPass->clear();
        profileNewPassAgain->clear();
    });

    buttonsHBox->addStretch();
    buttonsHBox->addWidget(saveChangesButton);
    buttonsHBox->addWidget(canselChangesButton);

    profileRightVBox->addSpacing(80);
    profileRightVBox->addLayout(profileNameHBox);
    profileRightVBox->addSpacing(15);
    profileRightVBox->addLayout(profileEmailHBox);
    profileRightVBox->addSpacing(15);
    profileRightVBox->addLayout(profilePhoneHBox);
    profileRightVBox->addSpacing(15);
    profileRightVBox->addLayout(profileOldPassHBox);
    profileRightVBox->addSpacing(15);
    profileRightVBox->addLayout(profileNewPassHBox);
    profileRightVBox->addSpacing(15);
    profileRightVBox->addLayout(profileNewPassAgainHBox);
    profileRightVBox->addSpacing(15);
    profileRightVBox->addLayout(buttonsHBox);
    profileRightVBox->addStretch();

    profileHBox->addLayout(profileLeftVBox);
    profileHBox->addLayout(profileRightVBox);
    profileHBox->addStretch();

    rightVBox->addLayout(profileHBox);
}

// поиск
void MainWindow::search(QString str)
{
    // удаление всех потенциальных друзей (если есть)
    for(int i = 0; i < potentialFriendsWidgets.size(); i++)
    {
        for(int j = 0; j < friendsVBox->count(); j++)
        {
            if(friendsVBox->itemAt(j)->widget() == potentialFriendsWidgets.at(i))
            {
                delete friendsVBox->itemAt(j)->widget();
            }
        }
    }
    potentialFriendsWidgets.clear();

    // если строка поиска пуста то показать всех друзей
    if(str == "")
    {
        for(int i = 0; i < friendsVBox->count(); i++)
        {
            if(dynamic_cast<QLabel* >(friendsVBox->itemAt(i)->widget()))
            {
                delete friendsVBox->itemAt(i)->widget();
            }
            else
            {
                friendsVBox->itemAt(i)->widget()->show();
            }
        }
        return;
    }

    // скрыть всех друзей
    for(int i = 0; i < friendsVBox->count(); i++)
    {
        if(dynamic_cast<QLabel* >(friendsVBox->itemAt(i)->widget()))
        {
            delete friendsVBox->itemAt(i)->widget();
        }
        else
        {
            friendsVBox->itemAt(i)->widget()->hide();
        }
    }

    // показать только тех друзей, которые удовлетворяю требованию поиска
    for(int i = 0; i < friendsVBox->count(); i++)
    {
        if(dynamic_cast<FriendWidget* >(friendsVBox->itemAt(i)->widget())->
                getName().toLower().indexOf(str.toLower()) != -1)
        {
            friendsVBox->itemAt(i)->widget()->show();
        }
    }
    QLabel* line = new QLabel(tr("potential friends"));
    line->setAlignment(Qt::AlignCenter);
    line->setFixedHeight(20);
    friendsVBox->addWidget(line, Qt::AlignTop);

    // запрос серверу на получение потенциальных друзей
    SlotSendToServer("/potentialFriends/" + str);
}

// показать/скрыть главное окно по нажатию на иконку в трее
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        if(!this->isVisible())
        {
            this->show();
        }
        else
        {
            this->hide();
        }
    }
}

// показать окно настроек
void MainWindow::clickedSettings()
{
    SettingsWidget* settingsW = new SettingsWidget;
    settingsW->setAttribute(Qt::WA_ShowModal, true);
    connect(settingsW, &SettingsWidget::updateSettings, [this](){
        emit(updateSettings());
    });
    settingsW->show();
}

// не принимать вызов
void MainWindow::noUpCalling(QString name, QString pass)
{
    // закрытие комнаты
    SlotSendToServer("/31/" + name + ":" + pass);
}

// создание комнаты
void MainWindow::CreateRoom()
{
    SlotSendToServer(nameRoom->text() + ":" + passRoom->text() + "/2/");

    isCreatedRoom = true;
    startRecord();
}

// открытие комнаты
void MainWindow::OpenRoom()
{
    SlotSendToServer(nameRoom->text() + ":" + passRoom->text() + "/3/");

    isOpenedRoom = true;
}

// слот для приема сообщение от сервера
void MainWindow::SlotReadyRead()
{
    QByteArray buffer;
    buffer = Socket->readAll();

    QDataStream in(buffer);

    QString str;
    in >> str;

    //qDebug() << "read:  " << str;

    if(buffer.indexOf("/camera/") != -1)
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
    // начало показа веб-камеры другом
    else if(buffer.indexOf("/startRecordVideo/") != -1)
    {
        buffer.remove(0, 18);
        // P.S. buffer - это id отправителя
        turnVideoBroadcast(buffer.toInt(), true);
    }
    // окончание показа веб-камеры другом
    else if(buffer.indexOf("/stopRecordVideo/") != -1)
    {
        buffer.remove(0, 17);
        // P.S. buffer - это id отправителя
        turnVideoBroadcast(buffer.toInt(), false);
    }
    // начало звонка
    else if(str.indexOf("/9/") != -1)
    {
        startRecord();
    }
    // добавление друзей в layout
    else if(str.indexOf("/5/") != -1)
    {
        str.remove("/5/");

        gettingFriends(str, "friend");
    }
    // добавление последних вызовов в layout
    else if(str.indexOf("/recent/") != -1)
    {
        str.remove("/recent/");

        gettingFriends(str, "recent");
    }
    // добавление потенциальных друзей в layout
    else if(str.indexOf("/potentialFriends/") != -1)
    {
        str.remove("/potentialFriends/");

        gettingFriends(str, "potentialFriends");
    }
    // добавление нового последнего вызова в layout
    else if(str.indexOf("/newRecent/") != -1)
    {
        str.remove("/newRecent/");

        gettingFriends(str, "newRecent");
    }
    // добавление запроса в друзья в layout
    else if(str.indexOf("/inviteToFriend/") != -1)
    {
        str.remove("/inviteToFriend/");
        gettingFriends(str, "inviteToFriend");
    }
    // согласие друга на добавление в друзья
    else if(str.indexOf("/acceptInviteToFriend/") != -1)
    {
        str.remove("/acceptInviteToFriend/");

        for(int i = 0; i < potentialFriendsWidgets.size(); i++)
        {
            if(potentialFriendsWidgets.at(i)->getId() == str)
            {
                potentialFriendsWidgets.at(i)->setIsEnableInviteToFriend(false);
                potentialFriendsWidgets.at(i)->setIsAcceptFriendInvitation("false");
                potentialFriendsWidgets.at(i)->setIsFriend(true);
                friendWidgets.append(potentialFriendsWidgets.at(i));
                potentialFriendsWidgets.removeAt(i);
                friendsVBox->insertWidget(friendsVBox->count() - 1, friendWidgets.back(), Qt::AlignTop);
                clickedFriendWidget(friendWidgets.back());
                return;
            }
        }
    }
    // получение истории сообщений
    else if(str.indexOf("/getMessages/") != -1)
    {
        str.remove("/getMessages/");

        static QDate lastDate;
        bool lastDateBool = false;
        bool first = false;
        bool isDownloadFile;

        QStringList list = str.split("/!/");

        for(int i = 0; i < list.count() - 1; i++)
        {
            isDownloadFile = false;
            QStringList listMessage = list[i].split("!");

            QString idFriend = listMessage[0];
            QString idMessage = listMessage[1];
            QString date = listMessage[2];
            QString time = listMessage[3];
            QString idChat = listMessage[4];
            QString message = listMessage[5];
            QString status = listMessage[6];

            if(numberBlockMessage != "1"
                    && lastDate == QDate::fromString(date,"dd MM yyyy") && lastDateBool == false)
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
                lastDate = QDate::fromString(date,"dd MM yyyy");
                lastDateBool = true;
            }

            if(message == "/beginningCall/")
            {
                if(idChat == id)
                {
                    message = name + " " + tr("started a conversation");
                }
                else
                {
                    message = friendInf->getName() + " " + tr("started a conversation");
                }
            }

            if(message.indexOf("/downloadFile/") != -1)
            {
                message.remove("/downloadFile/");
                isDownloadFile = true;
            }

            if(message.indexOf("/endingCall/") != -1)
            {
                message.remove(0, 12);

                message = tr("The call is complete. Duration: ") + message;
            }

            if(idFriend == friendInf->getId() || idFriend == id)
            {
                Message::side sideMess;
                if(idChat == id)
                {
                    sideMess = Message::right;
                }
                else
                {
                    sideMess = Message::left;
                    if(status == "1")
                    {
                        friendInf->readUnreadMessages();
                        SlotSendToServer("/readUnreadMessages/" + idMessage + "!"  + friendInf->getId());
                        QThread::msleep(15);
                    }
                }

                Message* m = new Message(message, time, sideMess);
                if(numberBlockMessage == "1")
                {
                    if(messageVBox->count() == 1 || Date != QDate::fromString(date,"dd MM yyyy"))
                    {
                        messageVBox->addWidget(new QLabel(
                                                   QDate::fromString(date,"dd MM yyyy").toString("dd MMMM yyyy")));
                        Date = QDate::fromString(date,"dd MM yyyy");
                    }
                    messageWidgets.append(m);
                    messageVBox->addWidget(m);
                }
                else
                {
                    if(Date != QDate::fromString(date,"dd MM yyyy"))
                    {
                        if(first == true)
                        {
                            messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                            Date = QDate::fromString(date,"dd MM yyyy");
                            scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                                      + 33);
                            first = false;
                        }
                        else if(first == false)
                        {
                            if(lastDate == Date)
                            {
                                messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                                Date = QDate::fromString(date,"dd MM yyyy");
                                scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                                          + 33);
                                first = true;
                            }
                            else
                            {
                                Date = QDate::fromString(date,"dd MM yyyy");
                                first = true;
                            }
                        }
                    }
                    messageWidgets.push_front(m);
                    messageVBox->insertWidget(0, m);
                    scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                              + m->sizeMessageL() + 20);
                }
                // если этот сообщение, загрузка файла, то создаем сигнал нажатия
                // на это сообщение и скачиваем файл с сервера
                if(isDownloadFile == true)
                {
                    connect(m, &Message::clicked, [this, message, idChat](){
                        FileTransfer* fileTransfer = new FileTransfer(this->id, this->identificationNumber,
                                                                      "downloadFile", idChat + "!" + message);
                        fileTransfer->start();
                    });
                }
            }
            // скролл до конца вниз
            if(isScrolling == true)
            {
                QTimer::singleShot(100, this, [this](){
                    scrollarea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);});
            }

            // добавление даты последнего сообщения на самый верх
            if(numberBlockMessage != "1" && i + 1 == list.count() - 1)
            {
                Date = QDate::fromString(date,"dd MM yyyy");
                lastDate = Date;
                messageVBox->insertWidget(0, new QLabel(Date.toString("dd MMMM yyyy")));
                scrollarea->verticalScrollBar()->setValue(scrollarea->verticalScrollBar()->value()
                                                          + 33);
            }
        }
    }
    // получение нового сообщения
    else if(str.indexOf("/newMessage/") != -1)
    {
        str.remove("/newMessage/");

        QStringList list = str.split("!");

        QString idSender = list[0];
        QString idMessage = list[1];
        QString date = list[2];
        QString time = list[3];
        QString idChat = list[4];
        QString message = list[5];
        QString status;

        bool isDownloadFile = false;

        if(list.count() == 7)
        {
            status = list[6];
        }

        if(message == "/beginningCall/")
        {
            if(idSender == id)
            {
                addRecent(idChat);
                message = name + " " + tr("started a conversation");
            }
            else
            {
                addRecent(idSender);
                message = friendInf->getName() + " " + tr("started a conversation");
            }
        }

        if(message.indexOf("/endingCall/") != -1)
        {
            message.remove(0, 12);

            message = tr("The call is complete. Duration: ") + message;
        }

        if(message.indexOf("/downloadFile/") != -1)
        {
            message.remove("/downloadFile/");
            isDownloadFile = true;
        }

        if(friendInf != nullptr && (friendInf->getId() == idSender || id == idSender))
        {
            if(Date != QDate::fromString(date,"dd MM yyyy"))
            {
                Date = QDate::fromString(date,"dd MM yyyy");
                messageVBox->addWidget(new QLabel(Date.toString("dd MMMM yyyy")));
            }
            if(idChat != id)
            {
                Message* m = new Message(message, time, Message::right);
                messageWidgets.append(m);
                messageVBox->addWidget(m);
                if(isDownloadFile == true)
                {
                    connect(m, &Message::clicked, [this, message, idSender](){
                        FileTransfer* fileTransfer = new FileTransfer(this->id, this->identificationNumber,
                                                                      "downloadFile", idSender + "!" + message);
                        fileTransfer->start();
                    });
                }
            }
            else
            {
                Message* m = new Message(message, time, Message::left);
                messageWidgets.append(m);
                messageVBox->addWidget(m);
                if(isDownloadFile == true)
                {
                    connect(m, &Message::clicked, [this, message, idSender](){
                        FileTransfer* fileTransfer = new FileTransfer(this->id, this->identificationNumber,
                                                                      "downloadFile", idSender + "!" + message);
                        fileTransfer->start();
                    });
                }
            }
            if(status == "1")
            {
                SlotSendToServer("/readUnreadMessages/" + idMessage + "!"  + friendInf->getId());
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
                if(friendWidgets[i]->getId() == idSender)
                {
                    friendWidgets[i]->newUnreadMessages();
                    break;
                }
            }
        }
        // проигрывание уведомления
        if(idSender != id)
        {
            player->play();
        }
    }
    // попытка звонка друга пользователю
    else if(str.indexOf("/29/") != -1)
    {
        str.remove("/29/");

        QStringList list = str.split("!");

        QString nameRoom = list[0];

        calling* callWidget = new calling(nameRoom,  list[1], this);
        callWidget->show();
    }
    // обновление статуса друга
    else if(str.indexOf("/33/") != -1)
    {
        str.remove("/33/");
        QStringList list = str.split(":");

        foreach (FriendWidget* friendW, friendWidgets)
        {
            if(friendW->getId() == list[0])
            {
                friendW->updateStatus(list[1]);
                if(friendInf != nullptr && friendInf == friendW)
                {
                    dynamic_cast<QLabel*>(mainScreenWithButtons->children().back())->setText(list[1]);
                }
                // если друг выщел из сети, но разговор ещё был, завершаем разговор
                if(friendW->getCallStatus() == true)
                {
                    clickedFriendWidget(friendW);
                    endCall();
                }
                if(friendW->getIsTryingCall() == "true")
                {
                    outOfTheRoom();
                }
            }
        }

        // меняем статус друга в списке
        foreach (FriendWidget* friendW, recentWidgets)
        {
            if(friendW->getId() == list[0])
            {
                friendW->updateStatus(list[1]);
                if(friendInf != nullptr && friendInf == friendW)
                {
                    dynamic_cast<QLabel*>(mainScreenWithButtons->children().back())->setText(list[1]);
                }
            }
        }

    }
    // начало звонка
    else if(str.indexOf("/beginnigCall/") != -1)
    {
        str.remove("/beginnigCall/");

        // обновление статуса начала звонка
        for(int i = 0; i < friendWidgets.count(); i++)
        {
            if(friendWidgets[i]->getId() == str)
            {
                friendWidgets[i]->setCallStatus(true);
                clickedFriendWidget(friendWidgets[i]);
            }
        }

        // обновление статуса начала звонка
        for(int i = 0; i < recentWidgets.count(); i++)
        {
            if(recentWidgets[i]->getId() == str)
            {
                recentWidgets[i]->setCallStatus(true);
            }
        }

        // если это был виде звонок, то включить веб-камеру
        if(isVideoCall)
        {
            clickedVideoButton();
        }

        friendInf->setIsTryingCall("false");
        isStartedCall = false;
        callPlayer->stop();

        // отправка другу сообщение о начале звонка
        SlotSendToServer("/message/" + friendInf->getId() + "!" + "/beginningCall/");
    }
    // закрытие комнаты
    else if(buffer.indexOf("/outoftheroom/") != -1)
    {
        outOfTheRoom();
    }
    // окончание звонка
    else if(str.indexOf("/endingCall/") != -1)
    {
        outOfTheRoom();
    }
    // обновление информации о друге(имя и т.д.)
    else if(str.indexOf("/updateFriendInfo/") != -1)
    {
        str.remove("/updateFriendInfo/");
        QStringList list = str.split("!");

        foreach (FriendWidget* friendW, friendWidgets)
        {
            if(friendW->getId() == list[0])
            {
                friendW->updateInfo(list[1]);
                if(friendInf != nullptr && friendInf == friendW)
                {
                    dynamic_cast<QLabel*>(mainScreenWithButtons->children().at
                                          (mainScreenWithButtons->children().count() - 2))->setText(list[1]);
                }
            }
        }
    }
    // обновление иконки друга(скачивание её с сервера)
    else if(str.indexOf("/updateFriendIcon/") != -1)
    {
        str.remove("/updateFriendIcon/");

        QStringList list = str.split("!");

        foreach (FriendWidget* friendW, friendWidgets)
        {
            if(friendW->getId() == list[0])
            {
                friendW->downloadNewIcon(list[1]);
            }
        }
    }
    // обновление иконки пользователя(скачивание её с сервера)
    else if(str.indexOf("/updateMainIcon/") != -1)
    {
        str.remove("/updateMainIcon/");

        profileWidget->updateIconName(str);
        iconName = str;

        FileTransfer* fileTransfer = new FileTransfer(id, identificationNumber,
                                                      "downloadMainIcon", id + "!" + str);
        fileTransfer->start();
        connect(fileTransfer, SIGNAL(updateIconFriend()), profileWidget, SLOT(updateIcon()));
        connect(fileTransfer, &FileTransfer::updateIconFriend, [this](){
            if(this->profileIcon != nullptr)
            {
              this->profileIcon->setPixmap(QIcon("IconFriends/" + this->id
                                                 + "!" + this->iconName).pixmap(256, 256));
            }
        });
    }
    // ошибка неверного пароля
    else if(str.indexOf("/Incorrect password/") != -1)
    {
        QMessageBox::critical(NULL,tr("Error"), tr("Incorrect password!"));
    }
    // информации о правильном пароле
    else if(str.indexOf("/Password changed/") != -1)
    {
        QMessageBox::information(NULL,tr("Information"), tr("Password changed!"));
    }
    else
    {
        video += buffer;
    }
}

// отправление потока веб-камеры на сервер
void MainWindow::sendCamera(QByteArray buff)
{
    Socket->write(buff);
    Socket->flush();
}

// передача трансляции веб-камеры от друга
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

// включает/выключает трансляию веб-камеры
void MainWindow::turnVideoBroadcast(int idSender, bool onOff)
{
    // изменяем статус показа веб-камеры друга
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getId().toInt() == idSender)
        {
            friendWidgets.at(i)->setVideoStatus(onOff);
            break;
        }
    }

    for(int i = 0; i < recentWidgets.count(); i++)
    {
        if(recentWidgets.at(i)->getId().toInt() == idSender)
        {
            recentWidgets.at(i)->setVideoStatus(onOff);
            break;
        }
    }

    // добавляем окно
    if(friendInf->getId().toInt() == idSender)
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

// обновление информации пользователя
void MainWindow::updateInfo(QString info)
{
    if(info.indexOf("/name/") != -1)
    {
        info.remove("/name/");
        name = info;
        profileWidget->updateName(info);
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

// завершение звонка и выход из комнаты
void MainWindow::outOfTheRoom()
{
    // изменение статуса звонка
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true)
        {
            friendWidgets.at(i)->setCallStatus(false);
            friendWidgets.at(i)->setVideoStatus(false);
            clickedFriendWidget(friendWidgets.at(i));
        }
    }
    for(int i = 0; i < recentWidgets.count(); i++)
    {
        if(recentWidgets.at(i)->getCallStatus() == true)
        {
            recentWidgets.at(i)->setCallStatus(false);
            recentWidgets.at(i)->setVideoStatus(false);
        }
    }
    isOpenedRoom = false;
    isCreatedRoom = false;
    isStartedCall = false;
    callPlayer->stop();
    friendInf->setIsTryingCall("false");

    // завершаем трансляцию аудио и видео
    QMetaObject::invokeMethod(audio, "stopRecord", Qt::DirectConnection);
    QMetaObject::invokeMethod(webCam, "stopRecord", Qt::DirectConnection);
}

// аудио сервер подключен
void MainWindow::connectedAudio()
{
    isConnectedAudio = true;
}

// полная очистка layout'а
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

// создание виджета разговорного чата
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

    // изменение состояния микрофона
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

    // изменение состояния веб-камера
    connect(videoButton, SIGNAL(clicked(bool)), this, SLOT(clickedVideoButton()));

    QPushButton* callEndButton = new QPushButton(buttons);
    callEndButton->setIcon(QIcon(":/Icons/call_end_icon.png"));
    callEndButton->move(109, 15);
    callEndButton->resize(32, 32);

    // завершение звонка
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

// создание виджета простого чата
void MainWindow::createMainFriendWidget()
{
    QHBoxLayout* hBox = new QHBoxLayout;

    mainScreenWithButtons = new QWidget;
    mainScreenWithButtons->setMinimumSize(200, 110);

    QLabel* profileIconRightWidget = new QLabel(mainScreenWithButtons);
    profileIconRightWidget->setPixmap(friendInf->getProfileIcon().pixmap(64, 64));

    QLabel* profileNameRightWidget = new QLabel(mainScreenWithButtons);
    profileNameRightWidget->setText(friendInf->getName());
    profileNameRightWidget->move(70, 15);

    QLabel* profileStatusRightWidget = new QLabel(mainScreenWithButtons);
    profileStatusRightWidget->setText(friendInf->getStatus());
    profileStatusRightWidget->move(70, 35);

    QWidget* buttons = new QWidget;

    QPushButton* callButton = new QPushButton(buttons);
    callButton->setIcon(QIcon(":/Icons/call_icon.png"));
    callButton->move(15, 15);
    callButton->resize(32, 32);

    // начало звонка другу
    connect(callButton, &QPushButton::clicked, [this](){
        clickedCallButton();
        isVideoCall = false;
    });

    QPushButton* videoButton = new QPushButton(buttons);
    videoButton->setIcon(QIcon(":/Icons/video_on_icon.png"));
    videoButton->move(62, 15);
    videoButton->resize(32, 32);

    // начало видеозвонка другу
    connect(videoButton, &QPushButton::clicked, [this](){
        if(webCam->getIsCamera() == false)
        {
            QMessageBox::critical(NULL,tr("Error"), tr("Webcam is not connected!"));
            return;
        }
        clickedCallButton();
        isVideoCall = true;
    });

    hBox->addWidget(mainScreenWithButtons);
    hBox->addWidget(buttons);

    rightVBox->insertLayout(0, hBox);
}

// отлавливаем ивент закрытия комнаты
void MainWindow::closeEvent(QCloseEvent *event)
{
    // если выход был нажат не в меню иконки в трее, то просто скрыть окно
    if(isFinalClosing == false)
    {
        if(this->isVisible())
        {
            event->ignore();
            this->hide();
        }
    }
    // если нет, просто закрываем приложение
}

// создание виджета чата с другом
void MainWindow::clickedFriendWidget(FriendWidget *friendW)
{
    isScrolling = true;
    numberBlockMessage = "1";
    Date.setDate(1900, 1, 1);

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

    // создание виджета прокрутки
    scrollarea = new QScrollArea();
    connect(scrollarea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(requestNewMessage(int)));
    scrollarea->setWidgetResizable(true);
    scrollarea->setFrameStyle(QFrame::NoFrame);

    ChatWidget *messageWidget = new ChatWidget(id, identificationNumber, friendInf->getId());
    scrollarea->setWidget(messageWidget);

    // отправка другу файла
    connect(messageWidget, &ChatWidget::sendFriendFileMessage, [this](QString idFriend, QString nameFile){
        this->SlotSendToServer("/message/" + idFriend + "!" + "/downloadFile/" + nameFile);
    });

    messageVBox = new QVBoxLayout();
    messageVBox->addStretch();
    messageVBox->setSpacing(20);
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

    // если этот пользователе не друг, то кастомизируем окно
    if(friendW->getIsFriend() == false)
    {
        QPushButton* addFriendB = new QPushButton;
        addFriendB->setEnabled(friendW->getIsEnableInviteToFriend());
        QString qss = ("QPushButton{"
                                  "font-weight: 700;"
                                  "text-decoration: none;"
                                  "padding: .5em 2em;"
                                  "outline: none;"
                                  "border: 2px solid;"
                                  "border-radius: 1px;"
                                "} "
                                "QPushButton:!hover { background: rgb(255,255,255); }");
        addFriendB->setStyleSheet(qss);
        if(friendW->getIsAcceptFriendInvitation() == "true")
        {
            addFriendB->setText(tr("Accept friend invitation"));
            connect(addFriendB, &QPushButton::clicked, [this, friendW](){
                friendWidgets.append(friendW);
                friendW->setIsEnableInviteToFriend(false);
                friendW->setIsAcceptFriendInvitation("false");
                friendW->setIsFriend(true);
                SlotSendToServer("/acceptInviteToFriends/" + friendW->getId());
                clickedFriendWidget(friendW);
            });
            QHBoxLayout* buttonHBox = new QHBoxLayout;
            QPushButton* noAddFriendB = new QPushButton(tr("Do not accept friend invitation"));
            noAddFriendB->setStyleSheet(qss);
            connect(noAddFriendB, &QPushButton::clicked, [this, friendW](){
                for(int i = 0; i < friendsVBox->count(); i++)
                {
                    if(friendsVBox->itemAt(i)->widget() == friendW)
                    {
                        delete friendsVBox->itemAt(i)->widget();
                    }
                }
                SlotSendToServer("/doNotAcceptInviteToFriends/" + friendW->getId());
                clickedProfileWidget();
            });

            buttonHBox->addWidget(addFriendB);
            buttonHBox->addWidget(noAddFriendB);
            rightVBox->addLayout(buttonHBox);
        }
        else
        {
            addFriendB->setText(tr("Add as Friend"));
            connect(addFriendB, &QPushButton::clicked, [this, friendW, addFriendB](){
                friendW->setIsEnableInviteToFriend(false);
                addFriendB->setEnabled(false);
                SlotSendToServer("/inviteToFriends/" + friendW->getId());
            });
            rightVBox->addWidget(addFriendB);
        }
    }
    rightVBox->addWidget(scrollarea);
    rightVBox->addLayout(lineMessageBox);

    // отправка на сервер запрос на получение истории сообщений
    SlotSendToServer("/HistoryMessage/" + friendInf->getId() + "!" + numberBlockMessage);
}

// начало звонка другу
void MainWindow::clickedCallButton()
{
    // проверка этот аользователь друг или нет
    if(friendInf->getIsFriend() == false)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("This user is not your friend"));
        msgBox.exec();
        return;
    }
    // поключен ли аудио сервер
    if(isConnectedAudio == false)
    {
        return;
    }
    // есть ли ещё не завершённые звонки
    for(int i = 0; i < friendWidgets.count(); i++)
    {
        if(friendWidgets.at(i)->getCallStatus() == true || isStartedCall == true)
        {
            QMessageBox::critical(NULL,tr("Error"),
                                  tr("Unable to start a new call because another call has already been initiated"));
            return;
        }
    }
    // проверка в сети ли друг
    if(friendInf->getStatus() == tr("online"))
    {
        SlotSendToServer("/19/" + friendInf->getId());

        isCreatedRoom = true;

        startRecord();
        isStartedCall = true;
        callPlayer->play();
        friendInf->setIsTryingCall("true");
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Friend not online"));
        msgBox.exec();
    }
}

// отправка сообщения другу
void MainWindow::clickedSendMessageButton()
{
    // проверка этот пользователь друг
    if(lineMessage->text() != "" && friendInf->getIsFriend() == false)
    {
        lineMessage->clear();
        QMessageBox msgBox;
        msgBox.setText(tr("This user is not your friend"));
        msgBox.exec();
        return;
    }
    // проверка не пустое ли сообщение
    if(lineMessage->text() != "")
    {
        SlotSendToServer("/message/" + friendInf->getId() + "!" + lineMessage->text());
        lineMessage->clear();
    }
}

// запрос на получение более старых сообщений
void MainWindow::requestNewMessage(int value)
{
    if(value == 0 && messageVBox->count() >= 10)
    {
        isScrolling = false;
        numberBlockMessage = QString::number(numberBlockMessage.toInt() + 1);
        SlotSendToServer("/HistoryMessage/" + friendInf->getId() + "!" + numberBlockMessage);
    }
}
