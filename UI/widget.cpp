#include "widget.h"
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QDebug>
#include <QHostAddress>

Widget::Widget(QWidget *parent) : QWidget(parent)
{

    this->resize(500, 500);

    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audio = new QAudioInput(format, this);

    text = new QTextEdit(this);
    text->resize(300, 300);
    text->setReadOnly(true);

    inputText = new QLineEdit(this);
    inputText->resize(300, 25);
    inputText->move(0, 315);

    button = new QPushButton(this);
    button->resize(100, 50);
    button->setText("Send");
    button->move(400, 250);
    connect(button, SIGNAL(clicked()), SLOT(slotSendToServer()));

    Socket = new QTcpSocket(this);
    QHostAddress address("localhost");
    Socket->connectToHost(address, 7070);

    connect(Socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(Socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(Socket, SIGNAL(error(QAbstractSocket::SocketError)),
    this, SLOT(slotError(QAbstractSocket::SocketError)));

    device = audio->start();

    connect(device, SIGNAL(readyRead()), this, SLOT(slotSentAudioToServer()));
}

Widget::~Widget()
{

}

void Widget::slotConnected()
{
    qDebug() << "connected";
}

void Widget::slotReadyRead()
{

}

void Widget::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                         "The host was not found." :
                         err == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         err == QAbstractSocket::ConnectionRefusedError ?
                         "The connection was refused." :
                         QString(Socket->errorString())
                        );
        text->append(strError);
}

void Widget::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << quint16(0) << inputText->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    Socket->write(arrBlock);
    inputText->setText("");
}

void Widget::slotSentAudioToServer()
{
    QByteArray  arrBlock;
    arrBlock = device->read(8192);
    Socket->write(arrBlock);
}
