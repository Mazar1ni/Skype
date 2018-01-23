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
    Sox(QTcpSocket* soc, QObject* parent = nullptr);

signals:
    void sendSound(QByteArray);

public slots:
    void removeNoise();
    void startRecord();
    void stopRecord();

private:
    int count;
    MainWindow* window;
    QTcpSocket* socket;

    QAudioInput* AudioInput;
    QAudioFormat Format;
    QIODevice* DeviceInput = nullptr;
};

#endif // SOX_H
