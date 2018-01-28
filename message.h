#ifndef MESSAGE_H
#define MESSAGE_H

#include <QWidget>
#include <QLabel>

class QHBoxLayout;

class Message : public QWidget
{
    Q_OBJECT
public:
    enum side {left, right};
    explicit Message(QString message, QString time, side s, QWidget *parent = nullptr);

    int sizeMessageL();

    QString getMessage() const;

private:
    QLabel* messageL;
    QHBoxLayout* mainHBox;

};

#endif // MESSAGE_H
