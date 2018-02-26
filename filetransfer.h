#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QThread>
#include <QTcpSocket>
#include <QObject>
#include <QFileDialog>

class FileTransfer : public QThread
{
    Q_OBJECT
public:
    explicit FileTransfer(QString id, QString autNum, QString type, QString fileN = "", QObject *parent = nullptr);
    void run();
    void endFile(QByteArray buffer);
    void disconnect();

signals:
    void updateIconFriend();
    void sendFriendsUpdateIcon();
    void sendFriendFileMessage();

private slots:
    void slotReadyRead();
    void slotSendServer(QString str);
    void slotDataAcquisitionFriendIcon();
    void slotDataAcquisitionMainIcon();
    void slotDataTransferMainIcon();
    void slotUploadFile();
    void slotDownloadFile();

private:
    QString idUser;
    QString identificationNumber;
    QTcpSocket* socket;
    QString fileName;
    QFile* uploadFile;
};

#endif // FILETRANSFER_H
