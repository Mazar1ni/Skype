#include "settingsconnection.h"
#include <QLabel>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>

SettingsConnection::SettingsConnection(QWidget *parent) : QWidget(parent)
{
    // оформление окна
    this->setFixedSize(300, 100);
    this->setWindowTitle("Settings");

    // инициализация и оформление ввода IP
    IP = new QLineEdit(this);
    IP->resize(150, 20);
    IP->move(60, 20);
    QLabel* labelIP = new QLabel("IP", this);
    labelIP->move(20, 20);

    // инициализация и оформление ввода Port
    Port = new QLineEdit(this);
    Port->resize(150, 20);
    Port->move(60, 60);
    QLabel* labelPort = new QLabel("Port", this);
    labelPort->move(20, 60);

    // инициализация кнопки Save
    Save = new QPushButton(this);
    Save->resize(60, 30);
    Save->move(220, 30);
    Save->setText("Save");
    connect(Save, SIGNAL(clicked(bool)), this, SLOT(SaveSettings()));

    LoadSettings();
}

void SettingsConnection::SaveSettings()
{
    QRegExp RegIp("(\\d{,3}.){4}");
    QRegExp RegPort("(\\d{4})");
    if(RegIp.exactMatch(IP->text()) && RegPort.exactMatch(Port->text()))
    {
        QFile out(FileName);
        if(out.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&out);
            stream << IP->text() << ":" << Port->text();
            out.close();
            this->close();
        }
    }
    else
    {
        QMessageBox::critical(NULL,QObject::tr("Error"),tr("Please fill in all fields correctly!"));
    }
}

void SettingsConnection::LoadSettings()
{
    QFile in(FileName);
    if(in.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&in);
        QString text = stream.readAll();
        int pos = text.indexOf(":");
        IP->setText(text.left(pos));
        Port->setText(text.mid(pos+1));
        in.close();
    }
}
