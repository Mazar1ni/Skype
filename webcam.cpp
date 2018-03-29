#include "webcam.h"
#include "mainwindow.h"
#include <QCameraInfo>
#include <QBuffer>
#include <QDataStream>
#include <QSettings>

WebCam::WebCam(QObject *parent) : QAbstractVideoSurface(parent)
{
    // проверка есть ли вообще веб-камеры у пользователя
    if(QCameraInfo::availableCameras().count() > 0)
    {
        isCamera = true;
        settingPreferences();
    }
    else
    {
        isCamera = false;
    }
}

QList<QVideoFrame::PixelFormat> WebCam::supportedPixelFormats(QAbstractVideoBuffer::HandleType) const
{
    return QList<QVideoFrame::PixelFormat>()
        << QVideoFrame::Format_ARGB32
        << QVideoFrame::Format_ARGB32_Premultiplied
        << QVideoFrame::Format_RGB32
        << QVideoFrame::Format_RGB24
        << QVideoFrame::Format_RGB565
        << QVideoFrame::Format_RGB555
        << QVideoFrame::Format_ARGB8565_Premultiplied
        << QVideoFrame::Format_BGRA32
        << QVideoFrame::Format_BGRA32_Premultiplied
        << QVideoFrame::Format_BGR32
        << QVideoFrame::Format_BGR24
        << QVideoFrame::Format_BGR565
        << QVideoFrame::Format_BGR555
        << QVideoFrame::Format_BGRA5658_Premultiplied
        << QVideoFrame::Format_AYUV444
        << QVideoFrame::Format_AYUV444_Premultiplied
        << QVideoFrame::Format_YUV444
        << QVideoFrame::Format_YUV420P
        << QVideoFrame::Format_YV12
        << QVideoFrame::Format_UYVY
        << QVideoFrame::Format_YUYV
        << QVideoFrame::Format_NV12
        << QVideoFrame::Format_NV21
        << QVideoFrame::Format_IMC1
        << QVideoFrame::Format_IMC2
        << QVideoFrame::Format_IMC3
        << QVideoFrame::Format_IMC4
        << QVideoFrame::Format_Y8
        << QVideoFrame::Format_Y16
        << QVideoFrame::Format_Jpeg
        << QVideoFrame::Format_CameraRaw
        << QVideoFrame::Format_AdobeDng;
}

bool WebCam::present(const QVideoFrame &frame)
{
    if (frame.isValid() && isTransmit == true)
    {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        QImage image(cloneFrame.bits(),
                           cloneFrame.width(),
                           cloneFrame.height(),
                           QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat()));
        image = image.convertToFormat(QImage::Format_RGB888);
        image = image.transformed(QTransform().scale(1,-1).translate(0, image.width()));
        emit frameAvailable(image);
        cloneFrame.unmap();
        return true;
     }
     return false;
}

// начало звонка
void WebCam::startRecord()
{
    isTransmit = true;
}

// конец звонка
void WebCam::stopRecord()
{
    isTransmit = false;
}

void WebCam::handleFrame(QImage img)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << (int)img.format();
    ds << img.byteCount();
    ds << img.width();
    ds << img.height();
    ds.writeRawData((char*)img.bits(), img.byteCount());
    sendCamera("/camera/" + ba + "/end/");
}

// обновление настроек
void WebCam::updateSettings()
{
    bool temp = isTransmit;
    isTransmit = false;

    camera->deleteLater();
    settingPreferences();

    isTransmit = temp;
}

bool WebCam::getIsCamera() const
{
    return isCamera;
}

// установка настроек
void WebCam::settingPreferences()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QSettings settings("settings.conf", QSettings::IniFormat);
    settings.beginGroup("VideoSettings");
    foreach (const QCameraInfo &cameraInfo, cameras)
    {
        if(settings.value("Camera").toString() == cameraInfo.description())
        {
            camera = new QCamera(cameraInfo);
        }
    }
    if(camera == nullptr)
    {
        camera = new QCamera(QCameraInfo::defaultCamera());
    }
    settings.endGroup();
    camera->setCaptureMode(QCamera::CaptureVideo);

    camera->setViewfinder(this);
    connect(this, SIGNAL(frameAvailable(QImage)), this, SLOT(handleFrame(QImage)));
    camera->start();
}
