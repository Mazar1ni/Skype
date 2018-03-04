#ifndef CALLING_H
#define CALLING_H

#include <QWidget>
#include <QTimer>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include "mainwindow.h"

class calling : public QWidget
{
    Q_OBJECT

private slots:
    void upCalling();
    void noUpCalling();

public:
    calling(QString name, QString passRoom, MainWindow *parentW);

private:
    MainWindow* parent;
    QString nameRoom;
    QString pass;
    QTimer* callTimer;
    QMediaPlayer* player;
    QMediaPlaylist* playlist;
};

#endif // CALLING_H
