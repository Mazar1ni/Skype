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

    QTimer::singleShot(1000, [this](){
        this->slotSendServer("/connect/" + this->idUser + "!" + this->identificationNumber);
    });

    if(type == "uploadMainIcon")
    {
        QTimer::singleShot(2000, this, SLOT(slotDataTransferMainIcon()));
    }
    else if(type == "downloadFriendIcon")
    {
        QTimer::singleShot(2000, this, SLOT(slotDataAcquisitionFriendIcon()));
    }
    else if(type == "downloadMainIcon")
    {
        QTimer::singleShot(2000, this, SLOT(slotDataAcquisitionMainIcon()));
    }
    else if(type == "uploadFile")
    {
        QTimer::singleShot(2000, this, SLOT(slotUploadFile()));
    }
    else if(type == "downloadFile")
    {
        QTimer::singleShot(2000, this, SLOT(slotDownloadFile()));
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

void FileTransfer::disconnect()
{
    socket->close();
    quit();
    deleteLater();
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
        disconnect();
    }
    else if(buffer.indexOf("endFileIcon") != -1)
    {
        buffer.remove(buffer.size() - 12, 11);
        endFile(buffer);
        updateIconFriend();
        //QFile("IconFriends/" + fileName).remove();
        disconnect();
    }
    else if(buffer.indexOf("endFile") != -1)
    {
        buffer.remove(buffer.size() - 8, 7);
        endFile(buffer);
        //QFile("IconFriends/" + fileName).remove();
        disconnect();
    }
    else if(str.indexOf("/successTransferMainIcon/") != -1)
    {
        sendFriendsUpdateIcon();
        disconnect();
    }
    else if(str.indexOf("/successTransferFile/") != -1)
    {
        sendFriendFileMessage();
        disconnect();
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
    QString filePath = QFileDialog::getOpenFileName(0,
                                                 "Open File",
                                                 "",
                                                 "*.jpeg, *.jpg, *.png");

    QString nameFile = filePath.mid(filePath.lastIndexOf("/") + 1);

    slotSendServer("/informationFileIcon/" + nameFile);
    QThread::msleep(100);

    QFile file(filePath);
    file.open(QFile::ReadOnly);
    QByteArray dataFile = file.readAll();

    socket->write(dataFile + "mainIconEndFile");
}

void FileTransfer::slotUploadFile()
{
    QString nameFile = fileName.mid(fileName.lastIndexOf("/") + 1);

    slotSendServer("/informationUploadFile/" + nameFile);
    QThread::msleep(100);

    fileName.remove(0, 1);

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QByteArray dataFile = file.readAll();

    socket->write(dataFile + "uploadEndFile");
}

void FileTransfer::slotDownloadFile()
{
    QString directoryPath = QFileDialog::getExistingDirectory(0, "Directory Dialog", "");

    uploadFile = new QFile(directoryPath + "/" + fileName.mid(fileName.indexOf("!") + 1));
    uploadFile->open(QFile::WriteOnly);
    slotSendServer("/informationAcquisitionFile/" + fileName);
}
