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
    QHBoxLayout* registrationHBox = new QHBoxLayout;

    QVBoxLayout* registrationLeftVBox = new QVBoxLayout;
    QVBoxLayout* registrationRightVBox = new QVBoxLayout;

    QLabel* informationNameLabel = new QLabel("name must be between 6 and 255 characters");
    informationNameLabel->setStyleSheet("color: red;");
    informationNameLabel->setObjectName("false");
    QLabel* informationEmailLabel = new QLabel("invalid email");
    informationEmailLabel->setStyleSheet("color: red;");
    informationEmailLabel->setObjectName("false");
    QLabel* informationPhoneLabel = new QLabel("Incorrect telephone");
    informationPhoneLabel->setStyleSheet("color: red;");
    informationPhoneLabel->setObjectName("false");
    QLabel* informationLoginLabel = new QLabel("login must be between 6 and 30 characters");
    informationLoginLabel->setStyleSheet("color: red;");
    informationLoginLabel->setObjectName("false");
    informationDateLabel = new QLabel("invalid date");
    informationDateLabel->setStyleSheet("color: red;");
    informationDateLabel->setObjectName("false");
    QLabel* informationPassLabel = new QLabel("password must be between 6 and 30 characters");
    informationPassLabel->setStyleSheet("color: red;");
    informationPassLabel->setObjectName("false");
    QLabel* informationRepeatPassLabel = new QLabel("passwords must match");
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
            informationNameLabel->setText("name entered correctly");
            informationNameLabel->setStyleSheet("color: green;");
            informationNameLabel->setObjectName("true");
        }
        else
        {
            informationNameLabel->setText("name must be between 6 and 30 characters");
            informationNameLabel->setStyleSheet("color: red;");
            informationNameLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationEmailVBox = new QVBoxLayout;
    QLabel* registrationEmailLabel = new QLabel("Email Address");
    QLineEdit* registrationEmail = new QLineEdit;
    registrationEmail->setPlaceholderText("your valid email address");
    registrationEmailVBox->addStretch();
    registrationEmailVBox->addWidget(registrationEmailLabel);
    registrationEmailVBox->addWidget(registrationEmail);
    connect(registrationEmail, &QLineEdit::textChanged, [informationEmailLabel, registrationEmail](){
        QRegExp rx("^.+@.+[.]+.+$");
        if(rx.exactMatch(registrationEmail->text()))
        {
            informationEmailLabel->setText("email entered correctly");
            informationEmailLabel->setStyleSheet("color: green;");
            informationEmailLabel->setObjectName("true");
        }
        else
        {
            informationEmailLabel->setText("invalid email");
            informationEmailLabel->setStyleSheet("color: red;");
            informationEmailLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationLoginVBox = new QVBoxLayout;
    QLabel* registrationLoginLabel = new QLabel("Username");
    QLineEdit* registrationLogin = new QLineEdit;
    registrationLogin->setPlaceholderText("your unique username");
    registrationLoginVBox->addStretch();
    registrationLoginVBox->addWidget(registrationLoginLabel);
    registrationLoginVBox->addWidget(registrationLogin);
    connect(registrationLogin, &QLineEdit::textChanged, [informationLoginLabel, registrationLogin](){
        if(registrationLogin->text().size() > 6 && registrationLogin->text().size() < 30)
        {
            informationLoginLabel->setText("username entered correctly");
            informationLoginLabel->setStyleSheet("color: green;");
            informationLoginLabel->setObjectName("true");
        }
        else
        {
            informationLoginLabel->setText("login must be between 6 and 30 characters");
            informationLoginLabel->setStyleSheet("color: red;");
            informationLoginLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationPhoneVBox = new QVBoxLayout;
    QLabel* registrationPhoneLabel = new QLabel("Phone Number");
    QLineEdit* registrationPhone = new QLineEdit;
    registrationPhone->setPlaceholderText("your phone number");
    registrationPhoneVBox->addStretch();
    registrationPhoneVBox->addWidget(registrationPhoneLabel);
    registrationPhoneVBox->addWidget(registrationPhone);
    connect(registrationPhone, &QLineEdit::textChanged, [informationPhoneLabel, registrationPhone](){
        QRegExp rx("^\\++(\\d.*){3,}$");
        if(rx.exactMatch(registrationPhone->text()))
        {
            informationPhoneLabel->setText("phone entered correctly");
            informationPhoneLabel->setStyleSheet("color: green;");
            informationPhoneLabel->setObjectName("true");
        }
        else
        {
            informationPhoneLabel->setText("Incorrect telephone");
            informationPhoneLabel->setStyleSheet("color: red;");
            informationPhoneLabel->setObjectName("false");
        }
    });

    QVBoxLayout* registrationDateVBox = new QVBoxLayout;
    QLabel* registrationDateLabel = new QLabel("Date of Birth");
    QHBoxLayout* registrationDateHBox = new QHBoxLayout;
    dayComboBox = new QComboBox;
    for(int i = 1; i <= 31; i++)
    {
        dayComboBox->addItem(QString::number(i));
    }
    connect(dayComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkValidDate()));
    monthComboBox = new QComboBox;
    for(int i = 1; i <= 12; i++)
    {
        QLocale l;
        monthComboBox->addItem(l.monthName(i));
    }
    connect(monthComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkValidDate()));
    yearComboBox = new QComboBox;
    for(int i = 1900; i <= QDate::currentDate().year(); i++)
    {
         yearComboBox->addItem(QString::number(i));
    }
    connect(yearComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkValidDate()));
    registrationDateHBox->addWidget(dayComboBox);
    registrationDateHBox->addWidget(monthComboBox);
    registrationDateHBox->addWidget(yearComboBox);
    registrationDateVBox->addStretch();
    registrationDateVBox->addWidget(registrationDateLabel);
    registrationDateVBox->addLayout(registrationDateHBox);

    QVBoxLayout* registrationPassVBox = new QVBoxLayout;
    QLabel* registrationPassLabel = new QLabel("Password");
    QLineEdit* registrationPass = new QLineEdit;
    registrationPass->setPlaceholderText("select a strong password");
    registrationPassVBox->addStretch();
    registrationPassVBox->addWidget(registrationPassLabel);
    registrationPassVBox->addWidget(registrationPass);

    QVBoxLayout* registrationRepeatPassVBox = new QVBoxLayout;
    QLabel* registrationRepeatPassLabel = new QLabel("Re-peat Password");
    QLineEdit* registrationRepeatPass = new QLineEdit;
    registrationRepeatPass->setPlaceholderText("pe-reat password");
    registrationRepeatPassVBox->addStretch();
    registrationRepeatPassVBox->addWidget(registrationRepeatPassLabel);
    registrationRepeatPassVBox->addWidget(registrationRepeatPass);
    connect(registrationRepeatPass, &QLineEdit::textChanged,
            [informationRepeatPassLabel, registrationRepeatPass, registrationPass](){
        if(registrationRepeatPass->text() == registrationPass->text())
        {
            informationRepeatPassLabel->setText("password entered correctly");
            informationRepeatPassLabel->setStyleSheet("color: green;");
            informationRepeatPassLabel->setObjectName("true");
        }
        else
        {
            informationRepeatPassLabel->setText("passwords must match");
            informationRepeatPassLabel->setStyleSheet("color: red;");
            informationRepeatPassLabel->setObjectName("false");
        }
    });

    connect(registrationPass, &QLineEdit::textChanged,
            [informationPassLabel, registrationPass, registrationRepeatPass, informationRepeatPassLabel](){
        QRegExp rx("^([a-zA-Z0-9@*#]{6,30})$");
        if(rx.exactMatch(registrationPass->text()))
        {
            informationPassLabel->setText("password entered correctly");
            informationPassLabel->setStyleSheet("color: green;");
            if(registrationPass->text().indexOf("123") != -1 || registrationPass->text().indexOf("qwe") != -1)
            {
                informationPassLabel->setText("password entered correctly !!!WARNING!!! "
                                               "that your password is too weak");
            }
            informationPassLabel->setObjectName("true");
        }
        else
        {
            informationPassLabel->setText("password must be between 6 and 30 characters");
            informationPassLabel->setStyleSheet("color: red;");
            informationPassLabel->setObjectName("false");
        }

        if(registrationRepeatPass->text() == registrationPass->text())
        {
            informationRepeatPassLabel->setText("password entered correctly");
            informationRepeatPassLabel->setStyleSheet("color: green;");
            informationRepeatPassLabel->setObjectName("true");
        }
        else
        {
            informationRepeatPassLabel->setText("passwords must match");
            informationRepeatPassLabel->setStyleSheet("color: red;");
            informationRepeatPassLabel->setObjectName("false");
        }
    });

    QPushButton* btnCreate = new QPushButton;
    btnCreate->setText("Create an account");
    btnCreate->setMinimumSize(100, 30);
    connect(btnCreate, &QPushButton::clicked,
            [this, registrationRightVBox, registrationName, registrationEmail, registrationLogin,
            registrationPhone, registrationPass](){
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
            socket->connectToHost("37.230.116.56", 7070);
            connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
        }

        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

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

void Registration::checkValidDate()
{
    QDate date(yearComboBox->currentText().toInt(), monthComboBox->currentIndex() + 1,
               dayComboBox->currentText().toInt());
    if(date.isValid())
    {
        informationDateLabel->setText("date entered correctly");
        informationDateLabel->setStyleSheet("color: green;");
        informationDateLabel->setObjectName("true");
    }
    else
    {
        informationDateLabel->setText("invalid date");
        informationDateLabel->setStyleSheet("color: red;");
        informationDateLabel->setObjectName("false");
    }
}

void Registration::slotReadyRead()
{
    QDataStream in(socket);

    QString str;
    in >> str;

    if(str.indexOf("/loginExists/") != -1)
    {
        QMessageBox::critical(NULL,QObject::tr("Error"), "Such login already exists!");
    }
    else if(str.indexOf("/successfully/") != -1)
    {
        QMessageBox::information(NULL,QObject::tr("Successfully"), "You have successfully registered!");
        deleteLater();
    }
}
