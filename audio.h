#ifndef AUDIO_H
#define AUDIO_H

#include <QProcess>
#include <QAudioInput>
#include <QIODevice>
#include <QTcpSocket>
#include <QThread>
#include <QAudioOutput>
#include <QTimer>
#include <QUdpSocket>

class MainWindow;

class Audio : public QObject
{
    Q_OBJECT
public:
    Audio(QObject* parent = nullptr);
    int ApplyVolumeToSample(short iSample);
    void settingPreferences();
    void setFriendIpPort(QString ip, QString port);

signals:
    void outOfTheRoom();
    void sendToServer(QString);

public slots:
    void removeNoise();
    void startRecord();
    void stopRecord();
    void connectStunServer();
    void updateSettings();

private slots:
    void slotReadyReadUdp();

private:
    MainWindow* window;

    QAudioInput* AudioInput;
    QAudioFormat Format;
    QIODevice* DeviceInput = nullptr;
    QAudioOutput* AudioOutput;
    QIODevice* DeviceOutput = nullptr;
    int iVolume = 0;

    bool isTransmit = false;

    QString ip;
    QString port;

    QHostAddress destAddress;
    QString destPort;

    QUdpSocket* udpSocket;
};

#endif // AUDIO_H
