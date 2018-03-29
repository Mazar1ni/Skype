#include "audio.h"
#include "mainwindow.h"
#include <QSettings>

Audio::Audio(QObject *parent) : QObject(parent), buffer(14096, 0)
{
    settingPreferences();
}

int Audio::ApplyVolumeToSample(short iSample)
{
    return std::max(std::min(((iSample * iVolume) / 50) ,35535), -35535);
}

// слот для отправки сообщений серверу
void Audio::SlotSendToServer(QString str)
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << str;

    socket->write(arrBlock);
    socket->flush();
}

// установка настроек для микроона и динамиков (поумолчанию или из настроек пользвателя)
void Audio::settingPreferences()
{
    // настройка аудио
    Format.setSampleRate(4000);
    Format.setChannelCount(1);
    Format.setSampleSize(16);
    Format.setCodec("audio/pcm");
    Format.setByteOrder(QAudioFormat::LittleEndian);
    Format.setSampleType(QAudioFormat::UnSignedInt);

    QSettings settings("settings.conf", QSettings::IniFormat);

    settings.beginGroup("AudioSettings");

    QAudioDeviceInfo deviceInput;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        if(deviceInfo.deviceName() == settings.value("Microphone").toString())
        {
            deviceInput = deviceInfo;
            break;
        }
    }

    if(deviceInput.isNull())
    {
        deviceInput = QAudioDeviceInfo::defaultInputDevice();
    }

    QAudioDeviceInfo deviceOutput;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        if(deviceInfo.deviceName() == settings.value("Speakers").toString())
        {
            deviceOutput = deviceInfo;
            break;
        }
    }

    if(deviceOutput.isNull())
    {
        deviceOutput = QAudioDeviceInfo::defaultInputDevice();
    }

    AudioInput = new QAudioInput(deviceInput, Format);
    AudioOutput = new QAudioOutput(deviceOutput, Format);

    DeviceOutput = AudioOutput->start();

    iVolume = settings.value("VolumeMicrophone", 99).toInt();
    AudioOutput->setVolume(settings.value("VolumeSpeakers", 99).toInt());

    settings.endGroup();
}

// удаление шумов из аудио
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

// начало записи
void Audio::startRecord()
{
    DeviceInput = AudioInput->start();
    isTransmit = true;
    connect(DeviceInput, SIGNAL(readyRead()), this, SLOT(removeNoise()));
}

// конец записи
void Audio::stopRecord()
{
    isTransmit = false;
}

// соединение с аудио сервером
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

// обновление настроек
void Audio::updateSettings()
{
    bool temp = isTransmit;
    isTransmit = false;

    AudioInput->deleteLater();
    AudioOutput->deleteLater();

    DeviceInput = nullptr;
    DeviceOutput = nullptr;

    settingPreferences();

    isTransmit = temp;
}

// слот для приема сообщение от сервера
void Audio::slotReadyRead()
{
    QByteArray buffer;
    buffer.resize(socket->size());

    socket->read(buffer.data(), buffer.size());

    QDataStream in(buffer);

    QString str;
    in >> str;

    // получение аудио
    if(buffer.indexOf("/18/") != -1)
    {
        buffer.remove(0, 4);
        DeviceOutput->write(buffer);
    }
    // выход из комнаты и завершение звонка
    else if(str.indexOf("/outoftheroom/") != -1)
    {
        emit(outOfTheRoom());
    }
    // установлено соединение с аудио сервером
    else if(str.indexOf("/connected/") != -1)
    {
        emit(connectedAudio());
    }
}
