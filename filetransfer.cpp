#include "filetransfer.h"
#include <QDataStream>
#include <QTimer>
#include <algorithm>

FileTransfer::FileTransfer(QString id, QString autNum, QString type, QString fileN, QObject *parent)
    : QThread(parent), idUser(id), identificationNumber(autNum), fileName(fileN)
{
    socket = new QTcpSocket;
    socket->connectToHost("185.146.157.27", 7071);

    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    QTimer::singleShot(1000, [this](){
        this->slotSendServer("/connect/" + this->idUser + "!" + this->identificationNumber);
    });

    if(type == "uploadMainIcon")
    {
        slotDataTransferMainIcon();
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

void FileTransfer::endFile()
{
    uploadFile.close();
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
    else if(str.indexOf("/sizeFile/") != -1)
    {
        str.remove("/sizeFile/");
        sizeFile = str;
    }
    else
    {
        uploadFile.write(buffer);

        currentSizeFile += buffer.size();

        if(currentSizeFile == sizeFile.toInt())
        {
            if(typeFile == "endFileIcon")
            {
                endFile();
                updateIconFriend();
                //QFile("IconFriends/" + fileName).remove();
                disconnect();
            }
            else if(typeFile == "endFile")
            {
                endFile();
                //QFile("IconFriends/" + fileName).remove();
                disconnect();
            }
        }
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
    uploadFile.setFileName("IconFriends/" + fileName);
    uploadFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    slotSendServer("/informationFileAcquisitionIcon/" + fileName);

    typeFile = "endFileIcon";
    currentSizeFile = 0;
}

void FileTransfer::slotDataAcquisitionMainIcon()
{
    uploadFile.setFileName("IconFriends/" + fileName);
    uploadFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    slotSendServer("/informationFileAcquisitionIcon/" + fileName);

    typeFile = "endFileIcon";
    currentSizeFile = 0;
}

void FileTransfer::slotDataTransferMainIcon()
{
    QString filePath = QFileDialog::getOpenFileName(0,
                                                 "Open File",
                                                 "",
                                                 "*.jpeg *.jpg *.png");

    QString nameFile = filePath.mid(filePath.lastIndexOf("/") + 1);

    QFile file(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    QByteArray dataFile = file.readAll();

    slotSendServer("/informationFileIcon/" + nameFile + "~" + QString::number(dataFile.size()));
    QThread::msleep(100);

    socket->write(dataFile);
    socket->flush();
}

void FileTransfer::slotUploadFile()
{
    QString nameFile = fileName.mid(fileName.lastIndexOf("/") + 1);

    fileName.remove(0, 1);

    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    QByteArray dataFile = file.readAll();

    slotSendServer("/informationUploadFile/" + nameFile + "~" + QString::number(dataFile.size()));
    QThread::msleep(100);

    socket->write(dataFile);
    socket->flush();
}

void FileTransfer::slotDownloadFile()
{
    QString directoryPath = QFileDialog::getExistingDirectory(0, "Directory Dialog", "");

    uploadFile.setFileName(directoryPath + "/" + fileName.mid(fileName.indexOf("!") + 1));
    uploadFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    slotSendServer("/informationAcquisitionFile/" + fileName);

    typeFile = "endFile";
    currentSizeFile = 0;
}
