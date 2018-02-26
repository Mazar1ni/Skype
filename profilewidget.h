#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QLabel>

class ProfileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileWidget(QString n, QString iconN, QString i, QString identNumber, QWidget *parent = nullptr);
    void mousePressEvent(QMouseEvent* event);
    void updateName(QString newName);
    void updateIconName(QString newName);

signals:
    clicked();
private slots:
    void updateIcon();

private:
    QString name;
    QString iconName;
    QString id;
    QString identifiacationNumber;
    QLabel* profileName;
    QLabel* profileIcon;
};

#endif // PROFILEWIDGET_H
