#ifndef UPDATER_H
#define UPDATER_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include <QLabel>
#include <QMovie>

class Updater : public QWidget
{
    Q_OBJECT
public:
    explicit Updater(QWidget *parent = nullptr);
    void downloadFile();
    void checkForExistenceFile(QString path);

signals:
    void endUpdate();

public slots:

private slots:
    void slotReadyRead();
    void slotSendToServer(QString mess);
    void disconnect();

private:
    QTcpSocket* socket;
    QFile* updateFile;
    QStringList updateFiles;
    QString mainPath;
    QLabel* infoLabel;
    QMovie movie;
    bool isConnected = false;
    QTimer* connectedTimer;

};

#endif // UPDATER_H
