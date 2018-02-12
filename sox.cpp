#include "sox.h"
#include "mainwindow.h"
#include <QFile>
#include <QtMultimedia/QAudioFormat>

Sox::Sox(QObject *parent) : QObject(parent)
{
    // настройка аудио
    Format.setSampleRate(4000);
    Format.setChannelCount(1);
    Format.setSampleSize(16);
    Format.setCodec("audio/pcm");
    Format.setByteOrder(QAudioFormat::LittleEndian);
    Format.setSampleType(QAudioFormat::UnSignedInt);

    AudioInput = new QAudioInput(Format, this);
}

void Sox::removeNoise()
{
    if(isTransmit == false)
    {
        return;
    }
    QByteArray  arrBlock;
    arrBlock = DeviceInput->readAll();

    QFile destinationFile;
    destinationFile.setFileName(".//sox/files/test.raw");
    destinationFile.open( QIODevice::WriteOnly);

    destinationFile.write(arrBlock);
    destinationFile.close();

    QString program = ".//sox/sox.exe";
    QStringList arguments;
    arguments << "-r" << "4000" << "-b" << "16" << "-c" << "1"
              << "-L" << "-e" << "signed-integer" << ".//sox/files/test.raw"
    << ".//sox/files/file.wav";
    QProcess process(this);
    process.start(program, arguments);

    if(process.waitForFinished())
    {
        process.kill();
        arguments.clear();
        arguments << ".//sox/files/file.wav"
                  << ".//sox/files/band.wav" << "bandpass" << "1000" << "2000";

        process.start(program, arguments);
    }

//    if(process->waitForFinished())
//    {
//        arguments.clear();
//        arguments << ".//sox/files/file.wav"
//                  << ".//sox/files/low.wav" << "lowpass" << "100";

//        process->start(program, arguments);
//    }


//    if(process->waitForFinished())
//    {
//        arguments.clear();
//        arguments << ".//sox/files/low.wav"
//                  << ".//sox/files/high.wav" << "highpass" << "600";

//        process->start(program, arguments);
//    }


//    if(process->waitForFinished())
//    {
//        arguments.clear();
//        arguments << ".//sox/files/high.wav"
//                  << ".//sox/files/high1.wav" << "vol" << "50";

//        process->start(program, arguments);
//    }

    if(process.waitForFinished())
    {
        process.kill();
        arguments.clear();
        arguments << ".//sox/files/band.wav" << "-r" << "4000" << "-b" << "16" << "-c" << "1"
                  << "-L" << "-e" << "signed-integer"
                  << ".//sox/files/clear.raw";

        process.start(program, arguments);
    }

    if(process.waitForFinished())
    {
        process.kill();
        process.deleteLater();
        QFile destinationFile;
        destinationFile.setFileName(".//sox/files/clear.raw");
        destinationFile.open( QIODevice::ReadOnly);

        QString n = "/18/";
        QByteArray buff = n.toUtf8();

        buff += destinationFile.readAll();
        destinationFile.close();
        destinationFile.deleteLater();
        //socket->write(buff);
        emit(sendSound(buff));
//        QMetaObject::invokeMethod(window, "sendSound", Qt::AutoConnection,
//                                  Q_ARG(QByteArray, buff));
    }
}

void Sox::startRecord()
{
    DeviceInput = AudioInput->start();
    isTransmit = true;
    connect(DeviceInput, SIGNAL(readyRead()), this, SLOT(removeNoise()));
}

void Sox::stopRecord()
{
    isTransmit = false;
}
