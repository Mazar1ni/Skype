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

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
    bool present(const QVideoFrame &frame);

signals:
    void frameAvailable(QImage frame);
    void sendCamera(QByteArray buff);

public slots:
    void startRecord();
    void stopRecord();
    void handleFrame(QImage img);

private:
    QCamera* camera;
    bool isTransmit = false;
    MainWindow* par;
};

#endif // WEBCAM_H
