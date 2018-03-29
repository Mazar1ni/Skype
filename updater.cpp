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
#include <QMessageBox>

Updater::Updater(QWidget *parent) : QWidget(parent)
{
    this->resize(400, 200);

    // подключение к серверу
    socket = new QTcpSocket;
    socket->connectToHost("37.230.116.56", 7072);

    connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    // создание надписи и гифки
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

    infoLabel->setText(tr("check for updates ..."));

    QSettings* settings = new QSettings("version.conf", QSettings::IniFormat);

    // отправка серверу версии программы
    QTimer::singleShot(5000, [this, settings](){
        slotSendToServer("/version/" + settings->value("version", "1.0.0.0").toString());
    });

    connectedTimer = new QTimer;

    connect(connectedTimer, &QTimer::timeout, [this](){
        // проверка есть ли соединение с сервером, если нет, появляется ошибка и программа закрывается
        if(!isConnected)
        {
            QMessageBox::critical(NULL,tr("Error"), tr("Server is temporarily unavailable!"));
            disconnect();
        }
    });

    connectedTimer->start(20000);
}

// скачивание нужных файлов с сервера
void Updater::downloadFile()
{
    // проверка есть ли ещё файлы для обновления
    if(!updateFiles.isEmpty())
    {
        updateFile = new QFile(updateFiles.first());
        // проверка создан ли такой файл и доступен ли он по тому пути, который передан
        if(!updateFile->open(QIODevice::WriteOnly))
        {
            QString str = updateFiles.first();
            updateFile->close();
            delete updateFile;
            QStringList list = str.split("/");

            QString path = "";
            QString name;

            // создаем все директории до этого файла
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
        // отправляем серверу, что хотим скачать этот файл
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

// проверка всех файлов программы, нужны ли они ещё
void Updater::checkForExistenceFile(QString path)
{
    QDir currentFolder(path);

    currentFolder.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    currentFolder.setSorting(QDir::Name);

    // получение списка всех файлов в это директории(path)
    QFileInfoList folderitems(currentFolder.entryInfoList());

    foreach (QFileInfo i_file, folderitems)
    {
        // если это директория, то вызываем ещё раз эту функцию, для это директории
        if(i_file.fileName().indexOf(".") == -1)
        {
            checkForExistenceFile(i_file.path() + "/" + i_file.fileName() + "/");
        }
        // если это файл, то отправляем запрос на сервер проверить этот файл
        else if(i_file.fileName().indexOf(".old") == -1)
        {
            path.remove(mainPath);
            slotSendToServer("/fileTest/" + path + i_file.fileName());
            QThread::msleep(15);
        }
    }
}

// слот для приема сообщение от сервера
void Updater::slotReadyRead()
{
    QByteArray buffer;
    buffer = socket->readAll();

    QDataStream in(buffer);

    QString str;
    in >> str;

    // версия с сервера совпадает с локальной версией
    if(str.indexOf("/match/") != -1)
    {
        connectedTimer->stop();
        endUpdate();
        disconnect();
    }
    // версия с сервера не совпадает с локальной версией
    else if(str.indexOf("/doNotMatch/") != -1)
    {
        connectedTimer->stop();
        infoLabel->setText(tr("download updates ..."));
    }
    // проверка не устарели ли файлы
    else if(str.indexOf("/fileTest/") != -1)
    {
        str.remove("/fileTest/");

        // последнее изменение файла с сервера
        QString lastModified = str.mid(str.indexOf("$") + 1);

        // путь до файла
        QString path = str.left(str.indexOf("$"));

        QFile testFile(path);
        // проверка есть ли такой файл
        if(testFile.open(QIODevice::ReadOnly))
        {
            QFileInfo fileInfo(testFile);
            QDateTime d;
            d = fileInfo.lastModified();
            // если файл с сервера создан раньше, чем локальный файл, то локальный файл необходимо заменить
            if(QDate::fromString(lastModified, "dd MM yyyy") >
                    QDate::fromString(d.toString("dd MM yyyy"), "dd MM yyyy"))
            {
                testFile.rename(testFile.fileName() + ".old");
                updateFiles.append(path);
            }
        }
        // если нет добавляем его в список для скачивания
        else
        {
            updateFiles.append(path);
        }
    }
    // скачивание нужных файлов
    else if(str.indexOf("/wellHow/") != -1)
    {
        str.remove("/wellHow/");

        downloadFile();
    }
    // удаление не нужных файлов
    else if(str.indexOf("/removeFile/") != -1)
    {
        str.remove("/removeFile/");

        QFile file(str);
        file.remove();

        slotSendToServer("/wellHow/");
    }
    // конец обновления, обновление локальной версии и перезапуск программы
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
    // получение конца файла
    else if(buffer.indexOf("endFile") != -1)
    {
        buffer.remove(buffer.size() - 8, 7);
        updateFile->write(buffer);
        updateFile->close();
        updateFile->deleteLater();
        downloadFile();
    }
    // получение промежутка файла
    else
    {
        updateFile->write(buffer);
    }
}

// слот для отправки сообщений серверу
void Updater::slotSendToServer(QString mess)
{
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << mess;

    socket->write(arrBlock);
    socket->flush();
}

// отключение от сервера и удаление объекта
void Updater::disconnect()
{
    socket->close();
    deleteLater();
}
