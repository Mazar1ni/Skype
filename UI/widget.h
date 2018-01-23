#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QTcpSocket>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>

class AudioInfo;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void slotConnected();
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError err);
    void slotSendToServer();
    void slotSentAudioToServer();

private:
    QAudioInput* audio;
    QTcpSocket* Socket;
    QAudioFormat format;
    QByteArray outputBytes;
    QIODevice* device;

    QTextEdit* text;
    QLineEdit* inputText;
    QPushButton* button;
};

#endif // WIDGET_H
