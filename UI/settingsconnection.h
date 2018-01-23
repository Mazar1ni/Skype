#ifndef SETTINGSCONNECTION_H
#define SETTINGSCONNECTION_H

#include <QString>
#include <QLineEdit>
#include <QPushButton>

class SettingsConnection : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsConnection(QWidget *parent = nullptr);

private slots:
    void SaveSettings();
    void LoadSettings();

private:
    QLineEdit* IP;
    QLineEdit* Port;
    QPushButton* Save;
    const QString FileName = "SettingsConnection.txt";

};

#endif // SETTINGSCONNECTION_H
