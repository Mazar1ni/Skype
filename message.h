#ifndef MESSAGE_H
#define MESSAGE_H

#include <QWidget>
#include <QLabel>

class Message : public QWidget
{
    Q_OBJECT
public:
    enum side {left, right};
    explicit Message(QString message, QString time, side s, QWidget *parent = nullptr);

    int sizeMessageL();

private:
     QLabel* messageL;

};

#endif // MESSAGE_H
