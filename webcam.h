#ifndef WEBCAM_H
#define WEBCAM_H

#include <QCamera>
#include <QAbstractVideoSurface>

class MainWindow;

class WebCam : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit WebCam(QObject *parent = nullptr);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType) const;
    bool present(const QVideoFrame &frame);
    bool getIsCamera() const;
    void settingPreferences();

signals:
    void frameAvailable(QImage frame);
    void sendCamera(QByteArray buff);

public slots:
    void startRecord();
    void stopRecord();
    void handleFrame(QImage img);
    void updateSettings();

private:
    QCamera* camera = nullptr;
    bool isTransmit = false;
    bool isCamera;
    MainWindow* par;
};

#endif // WEBCAM_H
