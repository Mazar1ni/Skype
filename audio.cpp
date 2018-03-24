#include "audio.h"
#include "mainwindow.h"
#include <QTimer>

Audio::Audio(QObject *parent) : QObject(parent), buffer(14096, 0)
{
    // настройка аудио
    Format.setSampleRate(8000);
    Format.setChannelCount(1);
    Format.setSampleSize(16);
    Format.setCodec("audio/pcm");
    Format.setByteOrder(QAudioFormat::LittleEndian);
    Format.setSampleType(QAudioFormat::UnSignedInt);

    AudioInput = new QAudioInput(Format, this);
    AudioOutput = new QAudioOutput(Format, this);
}

int Audio::ApplyVolumeToSample(short iSample)
{
    return std::max(std::min(((iSample * iVolume) / 50) ,35535), -35535);
}

void Audio::SlotSendToServer(QString str)
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << str;

    socket->write(arrBlock);
    socket->flush();
}

void Audio::removeNoise()
{
    if(isTransmit == false)
    {
        return;
    }

    qint64 len = AudioInput->bytesReady();

    if(len > 4096)
        len = 4096;
    DeviceInput->read(buffer.data(), len);

    short* resultingData = (short*)buffer.data();


    short *outdata=resultingData;
    outdata[0] = resultingData [0];

    int iIndex;
    for (iIndex=1; iIndex < len; iIndex++)
    {
        outdata[iIndex] = 0.333 * resultingData[iIndex] + (1.0 - 0.333) * outdata[iIndex-1];
    }

    for ( iIndex=0; iIndex < len; iIndex++ )
    {
        outdata[iIndex] = ApplyVolumeToSample(outdata[iIndex]);
    }

    QString n = "/18/";
    QByteArray buff = n.toUtf8();

    QByteArray out((char*)outdata, len);

    buff += out;

    socket->write(buff);
    socket->flush();
}

void Audio::startRecord()
{
    DeviceInput = AudioInput->start();
    DeviceOutput = AudioOutput->start();
    isTransmit = true;
    connect(DeviceInput, SIGNAL(readyRead()), this, SLOT(removeNoise()));
}

void Audio::stopRecord()
{
    isTransmit = false;
}

void Audio::connectServer(QString id, QString identificator)
{
    idUser = id;
    identificationNumber = identificator;

    socket = new QTcpSocket;
    socket->connectToHost("37.230.116.56", 7073);

    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    QTimer::singleShot(2000, [this](){
        SlotSendToServer("/connect/" + idUser + "!" + identificationNumber);
    });
}

void Audio::slotReadyRead()
{
    QByteArray buffer;
    buffer = socket->readAll();

    QDataStream in(buffer);

    QString str;
    in >> str;

    if(buffer.indexOf("/18/") != -1)
    {
        buffer.remove(0, 4);
        DeviceOutput->write(buffer);
    }
    else if(str.indexOf("/outoftheroom/") != -1)
    {
        emit(outOfTheRoom());
    }
    else if(str.indexOf("/connected/") != -1)
    {
        emit(connectedAudio());
    }
}
