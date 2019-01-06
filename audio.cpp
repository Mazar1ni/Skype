#include "audio.h"
#include "mainwindow.h"
#include <QSettings>
#include <QDebug>

Audio::Audio(QObject *parent) : QObject(parent)
{
    settingPreferences();
}

int Audio::ApplyVolumeToSample(short iSample)
{
    return std::max(std::min(((iSample * (iVolume + 50)) / 50) ,35535), -35535);
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
    Format.setSampleType(QAudioFormat::SignedInt);

    QSettings settings("settings.conf", QSettings::IniFormat);

    settings.beginGroup("AudioSettings");

    qDebug() << "Suported Input devices";
    QAudioDeviceInfo deviceInput;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        qDebug() << "----------------------------";
        qDebug() << "Device name: "             << deviceInfo.deviceName();
        qDebug() << "Supported channel count: "   << deviceInfo.supportedChannelCounts();
        qDebug() << "Supported Codec: "           << deviceInfo.supportedCodecs();
        qDebug() << "Supported byte order: "      << deviceInfo.supportedByteOrders();
        qDebug() << "Supported Sample Rate: "     << deviceInfo.supportedSampleRates();
        qDebug() << "Supported Sample Size: "     << deviceInfo.supportedSampleSizes();
        qDebug() << "Supported Sample Type: "     << deviceInfo.supportedSampleTypes();
        qDebug() << "Preferred Device settings:"  << deviceInfo.preferredFormat();
        qDebug() << "----------------------------";

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

    qDebug() << "Suported output devices";
    QAudioDeviceInfo deviceOutput;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        qDebug() << "----------------------------";
        qDebug() << "Device name: "             << deviceInfo.deviceName();
        qDebug() << "Supported channel count: "   << deviceInfo.supportedChannelCounts();
        qDebug() << "Supported Codec: "           << deviceInfo.supportedCodecs();
        qDebug() << "Supported byte order: "      << deviceInfo.supportedByteOrders();
        qDebug() << "Supported Sample Rate: "     << deviceInfo.supportedSampleRates();
        qDebug() << "Supported Sample Size: "     << deviceInfo.supportedSampleSizes();
        qDebug() << "Supported Sample Type: "     << deviceInfo.supportedSampleTypes();
        qDebug() << "Preferred Device settings:"  << deviceInfo.preferredFormat();
        qDebug() << "----------------------------";
        if(deviceInfo.deviceName() == settings.value("Speakers").toString())
        {
            deviceOutput = deviceInfo;
            break;
        }
    }

    if(deviceOutput.isNull())
    {
        deviceOutput = QAudioDeviceInfo::defaultOutputDevice();
    }

    AudioInput = new QAudioInput(deviceInput, Format);
    AudioOutput = new QAudioOutput(deviceOutput, Format);

    DeviceOutput = AudioOutput->start();

    iVolume = settings.value("VolumeMicrophone", 100).toInt();
    AudioInput->setVolume(settings.value("VolumeMicrophone", 100).toInt() / (qreal)100);
    AudioOutput->setVolume(settings.value("VolumeSpeakers", 100).toInt() / (qreal) 100);

    settings.endGroup();
}

void Audio::setFriendIpPort(QString ip, QString port)
{
    destAddress = QHostAddress(ip);
    destPort = port;
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

    QByteArray buffer(4096, 0);
    DeviceInput->read(buffer.data(), len);

    short* resultingData = (short*)buffer.data();

    short *outdata=resultingData;
    outdata[0] = resultingData [0];

    int iIndex;
    for (iIndex=1; iIndex < len; iIndex++)
    {
        outdata[iIndex] = 0.666 * resultingData[iIndex] + (1.0 - 0.666) * outdata[iIndex-1];
    }

    for ( iIndex=0; iIndex < len; iIndex++ )
    {
        outdata[iIndex] = ApplyVolumeToSample(outdata[iIndex]);
    }

    QString n = "/18/";
    QByteArray buff = n.toUtf8();

    QByteArray out((char*)outdata, len);

    DeviceOutput->write((char*)outdata, len);

    buff += out;

    udpSocket->writeDatagram(buff, destAddress, (uint16_t)destPort.toInt());
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
void Audio::connectStunServer()
{
    udpSocket = new QUdpSocket;
    udpSocket->bind(QHostAddress::Any);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(slotReadyReadUdp()));

    udpSocket->writeDatagram("/StunRequest/" + sizeof("/StunRequest/"), QHostAddress("185.146.157.27"), 3478);
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

void Audio::slotReadyReadUdp()
{
    QByteArray buffer;
    buffer.resize(udpSocket->pendingDatagramSize());
    QHostAddress *address = new QHostAddress();

    udpSocket->readDatagram(buffer.data(), buffer.size(), address);

    QDataStream in(&buffer, QIODevice::ReadOnly);

    QString str;
    in >> str;

    if(str.indexOf("/StunResponse/") != -1)
    {
        str.remove("/StunResponse/");
        QStringList list = str.split("!");

        ip = list[0];
        port = list[1];

        sendToServer("/IpPort/" + ip + "!" + port);
    }
    else if(buffer.indexOf("/18/") != -1)
    {
        buffer.remove(0, 4);

        DeviceOutput->write(buffer);
    }
}
