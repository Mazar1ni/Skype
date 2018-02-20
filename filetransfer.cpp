#include "filetransfer.h"
#include <QDataStream>
#include <QTimer>
#include <algorithm>

FileTransfer::FileTransfer(QString id, QString autNum, QString type, QString fileN, QObject *parent)
    : QThread(parent), idUser(id), identificationNumber(autNum), fileName(fileN)
{
    socket = new QTcpSocket;
    socket->connectToHost("localhost", (qint16)7071);

    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    if(type == "uploadMainIcon")
    {
        QTimer::singleShot(5000, this, SLOT(slotDataTransferMainIcon()));
    }
    else if(type == "downloadFriendIcon")
    {
        QTimer::singleShot(5000, this, SLOT(slotDataAcquisitionFriendIcon()));
    }
    else if(type == "downloadMainIcon")
    {
        QTimer::singleShot(5000, this, SLOT(slotDataAcquisitionMainIcon()));
    }
}

void FileTransfer::run()
{
    exec();
}

void FileTransfer::endFile(QByteArray buffer)
{
    uploadFile->write(buffer);
    uploadFile->close();
    uploadFile->deleteLater();
}

void FileTransfer::slotReadyRead()
{
    QByteArray buffer;
    buffer = socket->readAll();

    QDataStream in(buffer);

    QString str;
    in >> str;

    if(str.indexOf("/invalidData/") != -1)
    {
        deleteLater();
    }
    else if(buffer.indexOf("endFileIcon") != -1)
    {
        buffer.remove(buffer.size() - 12, 11);
        endFile(buffer);
        updateIconFriend();
        //QFile("IconFriends/" + fileName).remove();
        socket->close();
        quit();
        deleteLater();
    }
    else if(str.indexOf("/successTransferMainIcon/") != -1)
    {
        sendFriendsUpdateIcon();
        socket->close();
        quit();
        deleteLater();
    }
    else
    {
        uploadFile->write(buffer);
    }
}

void FileTransfer::slotSendServer(QString str)
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << str;

    socket->write(arrBlock);
    socket->flush();
}

void FileTransfer::slotDataAcquisitionFriendIcon()
{
    uploadFile = new QFile("IconFriends/" + fileName);
    uploadFile->open(QFile::WriteOnly);
    slotSendServer("/informationFileAcquisitionIcon/" + fileName);
}

void FileTransfer::slotDataAcquisitionMainIcon()
{
    uploadFile = new QFile("IconFriends/" + fileName);
    uploadFile->open(QFile::WriteOnly);
    slotSendServer("/informationFileAcquisitionIcon/" + fileName);
}

void FileTransfer::slotDataTransferMainIcon()
{
    slotSendServer("/connect/" + idUser + "!" + identificationNumber);
    QString filePath = QFileDialog::getOpenFileName(0,
                                                 "Open File",
                                                 "",
                                                 "*.jpeg, *.jpg, *.png");

    QString fileName = filePath.mid(filePath.lastIndexOf("/") + 1);

    slotSendServer("/informationFileIcon/" + fileName);
    QThread::msleep(100);

    QFile file(filePath);
    file.open(QFile::ReadOnly);
    QByteArray dataFile = file.readAll();

    socket->write(dataFile + "/mainIconEndFile/");
}
