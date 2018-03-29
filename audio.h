#ifndef AUDIO_H
#define AUDIO_H

#include <QProcess>
#include <QAudioInput>
#include <QIODevice>
#include <QTcpSocket>
#include <QThread>
#include <QAudioOutput>
#include <QTimer>

class MainWindow;

class Audio : public QObject
{
    Q_OBJECT
public:
    Audio(QObject* parent = nullptr);
    int ApplyVolumeToSample(short iSample);
    void SlotSendToServer(QString str);
    void settingPreferences();

signals:
    void sendSound(QByteArray);
    void outOfTheRoom();
    void connectedAudio();

public slots:
    void removeNoise();
    void startRecord();
    void stopRecord();
    void connectServer(QString id, QString identificator);
    void updateSettings();

private slots:
    void slotReadyRead();

private:
    MainWindow* window;

    QAudioInput* AudioInput;
    QAudioFormat Format;
    QIODevice* DeviceInput = nullptr;
    QAudioOutput* AudioOutput;
    QIODevice* DeviceOutput = nullptr;
    QByteArray buffer;
    int iVolume = 0;

    bool isTransmit = false;

    QString idUser;
    QString identificationNumber;
    QTcpSocket* socket;
};

#endif // AUDIO_H
