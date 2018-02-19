#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QLabel>

class ProfileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileWidget(QString name, QWidget *parent = nullptr);
    void mousePressEvent(QMouseEvent* event);
    void updateName(QString newName);

signals:
    clicked();
public slots:

private:
    QString name;
    QLabel* profileName;
};

#endif // PROFILEWIDGET_H
