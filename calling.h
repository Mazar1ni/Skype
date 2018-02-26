#ifndef CALLING_H
#define CALLING_H

#include <QWidget>
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
};

#endif // CALLING_H
