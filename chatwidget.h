#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QDropEvent>
#include <QUrl>

class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(QString i, QString identNumber, QString idFr, QWidget *parent = nullptr);

signals:
    void sendFriendFileMessage(QString, QString);

protected:
    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);

private:
    QString id;
    QString identificatorNumber;
    QString idFriend;
    QString nameFile;
};

#endif // CHATWIDGET_H
