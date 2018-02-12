#ifndef SOX_H
#define SOX_H

#include <QThread>
#include <QProcess>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QIODevice>
#include <QTcpSocket>

class MainWindow;

class Sox : public QObject
{
    Q_OBJECT
public:
    Sox(QObject* parent = nullptr);

signals:
    void sendSound(QByteArray);

public slots:
    void removeNoise();
    void startRecord();
    void stopRecord();

private:
    MainWindow* window;

    QAudioInput* AudioInput;
    QAudioFormat Format;
    QIODevice* DeviceInput = nullptr;

    bool isTransmit = false;
};

#endif // SOX_H
