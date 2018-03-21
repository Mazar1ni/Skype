#include "updater.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QThread>
#include <QProcess>
#include <QApplication>
#include <QSettings>
#include <QBoxLayout>
#include <QTimer>

Updater::Updater(QWidget *parent) : QWidget(parent)
{
    this->resize(400, 200);

    socket = new QTcpSocket;
    socket->connectToHost("37.230.116.56", 7072);

    connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    QVBoxLayout* mainHBox = new QVBoxLayout;

    infoLabel = new QLabel;
    infoLabel->setAlignment(Qt::AlignCenter);

    QLabel* downloadIndicator = new QLabel;
    downloadIndicator->setAlignment(Qt::AlignCenter);
    movie.setFileName(":/downloadIndicator.gif");
    downloadIndicator->setMovie(&movie);
    movie.start();

    mainHBox->addWidget(downloadIndicator, Qt::AlignCenter);
    mainHBox->addWidget(infoLabel, Qt::AlignCenter);

    this->setLayout(mainHBox);

    infoLabel->setText("check for updates ...");

    QSettings* settings = new QSettings("version.conf", QSettings::IniFormat);

    QTimer::singleShot(5000, [this, settings](){
        slotSendToServer("/version/" + settings->value("version", "1.0.0.0").toString());
    });
}

void Updater::downloadFile()
{
    if(!updateFiles.isEmpty())
    {
        updateFile = new QFile(updateFiles.first());
        if(!updateFile->open(QIODevice::WriteOnly))
        {
            QString str = updateFiles.first();
            updateFile->close();
            delete updateFile;
            QStringList list = str.split("/");

            QString path = "";
            QString name;

            for(int i = 0; i < list.count(); i++)
            {
                name = list[i];
                if(name.indexOf(".") != -1)
                {
                    updateFile = new QFile(str);
                    updateFile->open(QIODevice::WriteOnly);
                    break;
                }
                QDir(path).mkdir(name);
                path += name + "/";
            }
        }
        slotSendToServer("/downloadFile/" + updateFiles.first());
        updateFiles.removeFirst();
    }
    else
    {
        mainPath = QDir::currentPath();
        checkForExistenceFile(mainPath);

        QThread::msleep(30);

        slotSendToServer("/wellHow/");
    }
}

void Updater::checkForExistenceFile(QString path)
{
    QDir currentFolder(path);

    currentFolder.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    currentFolder.setSorting(QDir::Name);

    QFileInfoList folderitems(currentFolder.entryInfoList());

    foreach (QFileInfo i_file, folderitems)
    {
        if(i_file.fileName().indexOf(".") == -1)
        {
            checkForExistenceFile(i_file.path() + "/" + i_file.fileName() + "/");
        }
        else if(i_file.fileName().indexOf(".old") == -1)
        {
            path.remove(mainPath);
            slotSendToServer("/fileTest/" + path + i_file.fileName());
            QThread::msleep(15);
        }
    }
}

void Updater::slotReadyRead()
{
    QByteArray buffer;
    buffer = socket->readAll();

    QDataStream in(buffer);

    QString str;
    in >> str;

    if(str.indexOf("/match/") != -1)
    {
        endUpdate();
        disconnect();
    }
    else if(str.indexOf("/doNotMatch/") != -1)
    {
        infoLabel->setText("download updates ...");
    }
    else if(str.indexOf("/fileTest/") != -1)
    {
        str.remove("/fileTest/");

        QString lastModified = str.mid(str.indexOf("$") + 1);

        QString path = str.left(str.indexOf("$"));

        QFile testFile(path);
        if(testFile.open(QIODevice::ReadOnly))
        {
            QFileInfo fileInfo(testFile);
            QDateTime d;
            d = fileInfo.lastModified();
            if(QDate::fromString(lastModified, "dd MM yyyy") >
                    QDate::fromString(d.toString("dd MM yyyy"), "dd MM yyyy"))
            {
                testFile.rename(testFile.fileName() + ".old");
                updateFiles.append(path);
            }
        }
        else
        {
            updateFiles.append(path);
        }
    }
    else if(str.indexOf("/wellHow/") != -1)
    {
        str.remove("/wellHow/");

        downloadFile();
    }
    else if(str.indexOf("/removeFile/") != -1)
    {
        str.remove("/removeFile/");

        QFile file(str);
        file.remove();

        slotSendToServer("/wellHow/");
    }
    else if(str.indexOf("/endUpdate/") != -1)
    {
        str.remove("/endUpdate/");

        QSettings settings("version.conf", QSettings::IniFormat);
        settings.setValue("version", str);
        settings.sync();

        QProcess::startDetached("Skype.exe", QStringList() << "-clear");
        QApplication::quit();

        disconnect();
    }
    else if(buffer.indexOf("endFile") != -1)
    {
        buffer.remove(buffer.size() - 8, 7);
        updateFile->write(buffer);
        updateFile->close();
        updateFile->deleteLater();
        downloadFile();
    }
    else
    {
        updateFile->write(buffer);
    }
}

void Updater::slotSendToServer(QString mess)
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << mess;

    socket->write(arrBlock);
    socket->flush();
}

void Updater::disconnect()
{
    socket->close();
    deleteLater();
}
