#include "chatwidget.h"
#include "filetransfer.h"
#include <QMimeData>
#include <QDebug>

ChatWidget::ChatWidget(QString i, QString identNumber, QString idFr, QWidget *parent)
    : QWidget(parent), id(i), identificatorNumber(identNumber), idFriend(idFr)
{
    setAcceptDrops(true);
}

// событие отпускания стороннего файла на виджет чата
// при этом этот файл будет загружен на сервер
// и отправлено сообщение другу
void ChatWidget::dropEvent(QDropEvent *ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();
    foreach(QUrl url, urls)
    {
        FileTransfer* fileTransfer = new FileTransfer(id, identificatorNumber, "uploadFile", url.path());
        fileTransfer->start();
        nameFile = url.path().mid(url.path().lastIndexOf("/") + 1);
        connect(fileTransfer, &FileTransfer::sendFriendFileMessage, [this](){
            this->sendFriendFileMessage(this->idFriend, this->nameFile);
        });
    }
}

void ChatWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}
