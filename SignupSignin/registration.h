#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QTcpSocket>

class Registration : public QWidget
{
    Q_OBJECT
public:
    explicit Registration(QWidget *parent = nullptr);

private slots:
    void checkValidDate();
    void slotReadyRead();

private:
    QComboBox* dayComboBox;
    QComboBox* monthComboBox;
    QComboBox* yearComboBox;
    QLabel* informationDateLabel;
    QTcpSocket* socket;
    const QString FileName = "SettingsConnection.txt";
};

#endif // REGISTRATION_H
