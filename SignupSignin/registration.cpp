#include "registration.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QDate>
#include <QRegExp>
#include <QDebug>
#include <QPushButton>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>
#include <QHostAddress>

Registration::Registration(QWidget *parent) : QWidget(parent)
{
    this->resize(600, 400);
    socket = new QTcpSocket;
    // создание полей для регистрации
    QHBoxLayout* registrationHBox = new QHBoxLayout;

    QVBoxLayout* registrationLeftVBox = new QVBoxLayout;
    QVBoxLayout* registrationRightVBox = new QVBoxLayout;

    QLabel* informationNameLabel = new QLabel(tr("name must be between 6 and 255 characters"));
    informationNameLabel->setStyleSheet("color: red;");
    informationNameLabel->setObjectName("false");
    QLabel* informationEmailLabel = new QLabel(tr("invalid email"));
    informationEmailLabel->setStyleSheet("color: red;");
    informationEmailLabel->setObjectName("false");
    QLabel* informationPhoneLabel = new QLabel(tr("Incorrect telephone"));
    informationPhoneLabel->setStyleSheet("color: red;");
    informationPhoneLabel->setObjectName("false");
    QLabel* informationLoginLabel = new QLabel(tr("login must be between 6 and 30 characters"));
    informationLoginLabel->setStyleSheet("color: red;");
    informationLoginLabel->setObjectName("false");
    informationDateLabel = new QLabel(tr("invalid date"));
    informationDateLabel->setStyleSheet("color: red;");
    informationDateLabel->setObjectName("false");
    QLabel* informationPassLabel = new QLabel(tr("password must be between 6 and 30 characters"));
    informationPassLabel->setStyleSheet("color: red;");
    informationPassLabel->setObjectName("false");
    QLabel* informationRepeatPassLabel = new QLabel(tr("passwords must match"));
    informationRepeatPassLabel->setStyleSheet("color: red;");
    informationRepeatPassLabel->setObjectName("false");

    registrationRightVBox->addSpacing(80);
    registrationRightVBox->addWidget(informationNameLabel);
    registrationRightVBox->addSpacing(25);
    registrationRightVBox->addWidget(informationEmailLabel);
    registrationRightVBox->addSpacing(25);
    registrationRightVBox->addWidget(informationPhoneLabel);
    registrationRightVBox->addSpacing(25);
    registrationRightVBox->addWidget(informationLoginLabel);
    registrationRightVBox->addSpacing(25);
    registrationRightVBox->addWidget(informationDateLabel);
    registrationRightVBox->addSpacing(25);
    registrationRightVBox->addWidget(informationPassLabel);
    registrationRightVBox->addSpacing(25);
    registrationRightVBox->addWidget(informationRepeatPassLabel);
    registrationRightVBox->addSpacing(50);

    QVBoxLayout* registrationNameVBox = new QVBoxLayout;
    QLabel* registrationNameLabel = new QLabel("Name");
    QLineEdit* registrationName = new QLineEdit;
    registrationName->setPlaceholderText("your name");
    registrationNameVBox->addStretch();
    registrationNameVBox->addWidget(registrationNameLabel);
    registrationNameVBox->addWidget(registrationName);
    connect(registrationName, &QLineEdit::textChanged, [informationNameLabel, registrationName](){
        if(registrationName->text().size() > 6 && registrationName->text().size() < 30)
        {
            informationNameLabel->setText(tr("name entered correctly"));
            informationNameLabel->setStyleSheet("color: green;");
            informationNameLabel->setObjectName("true");
        }
        else
        {
            informationNameLabel->setText(tr("name must be between 6 and 30 characters"));
            informationNameLabel->setStyleSheet("color: red;");
            informationNameLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationEmailVBox = new QVBoxLayout;
    QLabel* registrationEmailLabel = new QLabel(tr("Email Address"));
    QLineEdit* registrationEmail = new QLineEdit;
    registrationEmail->setPlaceholderText(tr("your valid email address"));
    registrationEmailVBox->addStretch();
    registrationEmailVBox->addWidget(registrationEmailLabel);
    registrationEmailVBox->addWidget(registrationEmail);
    // проверка совпадает ли значение с образцом при помощи регулярных выражений
    connect(registrationEmail, &QLineEdit::textChanged, [informationEmailLabel, registrationEmail](){
        QRegExp rx("^.+@.+[.]+.+$");
        if(rx.exactMatch(registrationEmail->text()))
        {
            informationEmailLabel->setText(tr("email entered correctly"));
            informationEmailLabel->setStyleSheet("color: green;");
            informationEmailLabel->setObjectName("true");
        }
        else
        {
            informationEmailLabel->setText(tr("invalid email"));
            informationEmailLabel->setStyleSheet("color: red;");
            informationEmailLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationLoginVBox = new QVBoxLayout;
    QLabel* registrationLoginLabel = new QLabel(tr("Username"));
    QLineEdit* registrationLogin = new QLineEdit;
    registrationLogin->setPlaceholderText(tr("your unique username"));
    registrationLoginVBox->addStretch();
    registrationLoginVBox->addWidget(registrationLoginLabel);
    registrationLoginVBox->addWidget(registrationLogin);
    // проверка совпадает ли значение с образцом при помощи регулярных выражений
    connect(registrationLogin, &QLineEdit::textChanged, [informationLoginLabel, registrationLogin](){
        if(registrationLogin->text().size() > 6 && registrationLogin->text().size() < 30)
        {
            informationLoginLabel->setText(tr("username entered correctly"));
            informationLoginLabel->setStyleSheet("color: green;");
            informationLoginLabel->setObjectName("true");
        }
        else
        {
            informationLoginLabel->setText(tr("login must be between 6 and 30 characters"));
            informationLoginLabel->setStyleSheet("color: red;");
            informationLoginLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationPhoneVBox = new QVBoxLayout;
    QLabel* registrationPhoneLabel = new QLabel(tr("Phone Number"));
    QLineEdit* registrationPhone = new QLineEdit;
    registrationPhone->setPlaceholderText(tr("your phone number"));
    registrationPhoneVBox->addStretch();
    registrationPhoneVBox->addWidget(registrationPhoneLabel);
    registrationPhoneVBox->addWidget(registrationPhone);
    // проверка совпадает ли значение с образцом при помощи регулярных выражений
    connect(registrationPhone, &QLineEdit::textChanged, [informationPhoneLabel, registrationPhone](){
        QRegExp rx("^\\++(\\d.*){3,}$");
        if(rx.exactMatch(registrationPhone->text()))
        {
            informationPhoneLabel->setText(tr("phone entered correctly"));
            informationPhoneLabel->setStyleSheet("color: green;");
            informationPhoneLabel->setObjectName("true");
        }
        else
        {
            informationPhoneLabel->setText(tr("Incorrect telephone"));
            informationPhoneLabel->setStyleSheet("color: red;");
            informationPhoneLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationDateVBox = new QVBoxLayout;
    QLabel* registrationDateLabel = new QLabel(tr("Date of Birth"));
    QHBoxLayout* registrationDateHBox = new QHBoxLayout;
    dayComboBox = new QComboBox;
    for(int i = 1; i <= 31; i++)
    {
        dayComboBox->addItem(QString::number(i));
    }
    // проверка валидности даты
    connect(dayComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkValidDate()));
    monthComboBox = new QComboBox;
    for(int i = 1; i <= 12; i++)
    {
        QLocale l;
        monthComboBox->addItem(l.monthName(i));
    }
    // проверка валидности даты
    connect(monthComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkValidDate()));
    yearComboBox = new QComboBox;
    for(int i = 1900; i <= QDate::currentDate().year(); i++)
    {
         yearComboBox->addItem(QString::number(i));
    }
    // проверка валидности даты
    connect(yearComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkValidDate()));
    registrationDateHBox->addWidget(dayComboBox);
    registrationDateHBox->addWidget(monthComboBox);
    registrationDateHBox->addWidget(yearComboBox);
    registrationDateVBox->addStretch();
    registrationDateVBox->addWidget(registrationDateLabel);
    registrationDateVBox->addLayout(registrationDateHBox);

    QVBoxLayout* registrationPassVBox = new QVBoxLayout;
    QLabel* registrationPassLabel = new QLabel(tr("Password"));
    QLineEdit* registrationPass = new QLineEdit;
    registrationPass->setPlaceholderText(tr("select a strong password"));
    registrationPass->setEchoMode(QLineEdit::Password);
    registrationPass->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
    registrationPassVBox->addStretch();
    registrationPassVBox->addWidget(registrationPassLabel);
    registrationPassVBox->addWidget(registrationPass);

    QVBoxLayout* registrationRepeatPassVBox = new QVBoxLayout;
    QLabel* registrationRepeatPassLabel = new QLabel(tr("Re-peat Password"));
    QLineEdit* registrationRepeatPass = new QLineEdit;
    registrationRepeatPass->setPlaceholderText(tr("re-peat password"));
    registrationRepeatPass->setEchoMode(QLineEdit::Password);
    registrationRepeatPass->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
    registrationRepeatPassVBox->addStretch();
    registrationRepeatPassVBox->addWidget(registrationRepeatPassLabel);
    registrationRepeatPassVBox->addWidget(registrationRepeatPass);
    // проверка совпадает ли значение с образцом при помощи регулярных выражений
    connect(registrationRepeatPass, &QLineEdit::textChanged,
            [informationRepeatPassLabel, registrationRepeatPass, registrationPass](){
        if(registrationRepeatPass->text() == registrationPass->text())
        {
            informationRepeatPassLabel->setText(tr("password entered correctly"));
            informationRepeatPassLabel->setStyleSheet("color: green;");
            informationRepeatPassLabel->setObjectName("true");
        }
        else
        {
            informationRepeatPassLabel->setText(tr("passwords must match"));
            informationRepeatPassLabel->setStyleSheet("color: red;");
            informationRepeatPassLabel->setObjectName("false");
        }
    });

    // проверка совпадает ли значение с образцом при помощи регулярных выражений
    connect(registrationPass, &QLineEdit::textChanged,
            [informationPassLabel, registrationPass, registrationRepeatPass, informationRepeatPassLabel](){
        QRegExp rx("^([a-zA-Z0-9@*#]{6,30})$");
        if(rx.exactMatch(registrationPass->text()))
        {
            informationPassLabel->setText(tr("password entered correctly"));
            informationPassLabel->setStyleSheet("color: green;");
            if(registrationPass->text().indexOf("123") != -1 || registrationPass->text().indexOf("qwe") != -1)
            {
                informationPassLabel->setText(tr("password entered correctly !!!WARNING!!! "
                                               "that your password is too weak"));
            }
            informationPassLabel->setObjectName("true");
        }
        else
        {
            informationPassLabel->setText(tr("password must be between 6 and 30 characters"));
            informationPassLabel->setStyleSheet("color: red;");
            informationPassLabel->setObjectName("false");
        }

        if(registrationRepeatPass->text() == registrationPass->text())
        {
            informationRepeatPassLabel->setText(tr("password entered correctly"));
            informationRepeatPassLabel->setStyleSheet("color: green;");
            informationRepeatPassLabel->setObjectName("true");
        }
        else
        {
            informationRepeatPassLabel->setText(tr("passwords must match"));
            informationRepeatPassLabel->setStyleSheet("color: red;");
            informationRepeatPassLabel->setObjectName("false");
        }
    });

    QString qss = ("QPushButton{"
                              "font-weight: 700;"
                              "text-decoration: none;"
                              "padding: .5em 2em;"
                              "outline: none;"
                              "border: 2px solid;"
                              "border-radius: 1px;"
                            "} "
                            "QPushButton:!hover { background: rgb(255,255,255); }");
    QPushButton* btnCreate = new QPushButton;
    btnCreate->setStyleSheet(qss);
    btnCreate->setText(tr("Create an account"));
    btnCreate->setMinimumSize(100, 30);
    // формирование данных для отправки серверу
    connect(btnCreate, &QPushButton::clicked,
            [this, registrationRightVBox, registrationName, registrationEmail, registrationLogin,
            registrationPhone, registrationPass](){
        // проверка есть ли поля не удовлетворяющие требованиям
        for(int i = 0; i < registrationRightVBox->count(); i++)
        {
            if(registrationRightVBox->itemAt(i)->widget())
            {
                if(registrationRightVBox->itemAt(i)->widget()->objectName() == "false")
                {
                    return;
                }
            }
        }

        QDate date(yearComboBox->currentText().toInt(), monthComboBox->currentIndex() + 1,
                   dayComboBox->currentText().toInt());

        if(!socket->isOpen())
        {
            socket->connectToHost("185.146.157.27", 7070);
            connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
        }

        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

        // отправка данных пользователя на сервер
        QString mess = "/registration/" + registrationName->text() + "!" + registrationEmail->text() + "!"
                       + registrationLogin->text() + "!" + registrationPhone->text() + "!"
                       + registrationPass->text() + "!" + date.toString("dd.MM.yyyy");

        QByteArray  arrBlock;

        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out << mess;

        socket->write(arrBlock);
    });

    registrationLeftVBox->addSpacing(80);
    registrationLeftVBox->addLayout(registrationNameVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addLayout(registrationEmailVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addLayout(registrationPhoneVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addLayout(registrationLoginVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addLayout(registrationDateVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addLayout(registrationPassVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addLayout(registrationRepeatPassVBox);
    registrationLeftVBox->addSpacing(25);
    registrationLeftVBox->addWidget(btnCreate);

    registrationHBox->addLayout(registrationLeftVBox);
    registrationHBox->addSpacing(15);
    registrationHBox->addLayout(registrationRightVBox);

    this->setLayout(registrationHBox);
}

// првоерка валидности даты
void Registration::checkValidDate()
{
    QDate date(yearComboBox->currentText().toInt(), monthComboBox->currentIndex() + 1,
               dayComboBox->currentText().toInt());
    if(date.isValid())
    {
        informationDateLabel->setText(tr("date entered correctly"));
        informationDateLabel->setStyleSheet("color: green;");
        informationDateLabel->setObjectName("true");
    }
    else
    {
        informationDateLabel->setText(tr("invalid date"));
        informationDateLabel->setStyleSheet("color: red;");
        informationDateLabel->setObjectName("false");
    }
}

// слот для приема сообщение от сервера
void Registration::slotReadyRead()
{
    QDataStream in(socket);

    QString str;
    in >> str;

    // такой логин уже занят
    if(str.indexOf("/loginExists/") != -1)
    {
        QMessageBox::critical(NULL,tr("Error"), tr("Such login already exists!"));
    }
    // регистрация прошла успешно
    else if(str.indexOf("/successfully/") != -1)
    {
        QMessageBox::information(NULL,tr("Successfully"), tr("You have successfully registered!"));
        deleteLater();
    }
}
