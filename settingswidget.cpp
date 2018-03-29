#include "settingswidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QAudioDeviceInfo>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(700, 500);
    setWindowIcon(QIcon(":/Icons/skype_icon.ico"));

    QSettings settings("settings.conf", QSettings::IniFormat);

    // кастомизируем окно и устанавливаем значения (поумолчанию или из файла настроек)

    QWidget* generalWidget = new QWidget;
    QVBoxLayout* generalVBox = new QVBoxLayout;

    QLabel* infoGeneralLabel = new QLabel(tr("General settings: Basic settings for Skype"));

    QHBoxLayout* languageHBox = new QHBoxLayout;

    QLabel* interfaceLanguage = new QLabel(tr("Choice of interface language"));
    QComboBox* comboLanguage = new QComboBox;
    comboLanguage->setFixedWidth(200);
    comboLanguage->addItem("English");
    comboLanguage->addItem("Russian");

    settings.beginGroup("GeneralSettings");
    comboLanguage->setCurrentText(settings.value("Language", "English").toString());
    settings.endGroup();

    languageHBox->addWidget(interfaceLanguage);
    languageHBox->addWidget(comboLanguage);
    languageHBox->addStretch();

    generalVBox->addWidget(infoGeneralLabel);
    generalVBox->addSpacing(20);
    generalVBox->addLayout(languageHBox);
    generalVBox->addStretch();

    generalWidget->setLayout(generalVBox);

    QWidget* audioWidget = new QWidget;
    QVBoxLayout* audioVBox = new QVBoxLayout;

    QLabel* infoAudioLabel = new QLabel(tr("Sound setting: Set the sound on your computer"));

    QHBoxLayout* microphoneHBox = new QHBoxLayout;

    QLabel* microphone = new QLabel(tr("Microphone"));
    QComboBox* comboMicrophone= new QComboBox;
    comboMicrophone->setFixedWidth(200);
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        comboMicrophone->addItem(deviceInfo.deviceName());
    }
    comboMicrophone->setCurrentText(QAudioDeviceInfo::defaultInputDevice().deviceName());

    microphoneHBox->addWidget(microphone);
    microphoneHBox->addWidget(comboMicrophone);
    microphoneHBox->addStretch();

    QHBoxLayout* volumeMicrophoneHBox = new QHBoxLayout;

    QLabel* volumeM = new QLabel(tr("Volume"));
    QSlider* volumeSliderM = new QSlider(Qt::Horizontal);
    volumeSliderM->setValue(99);

    volumeMicrophoneHBox->addWidget(volumeM);
    volumeMicrophoneHBox->addWidget(volumeSliderM);
    volumeMicrophoneHBox->addStretch();

    QHBoxLayout* speakersHBox = new QHBoxLayout;

    QLabel* speakers = new QLabel(tr("Speakers"));
    QComboBox* comboSpeakers= new QComboBox;
    comboSpeakers->setFixedWidth(200);
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        comboSpeakers->addItem(deviceInfo.deviceName());
    }
    comboSpeakers->setCurrentText(QAudioDeviceInfo::defaultOutputDevice().deviceName());

    speakersHBox->addWidget(speakers);
    speakersHBox->addWidget(comboSpeakers);
    speakersHBox->addStretch();

    QHBoxLayout* volumeSpeakersHBox = new QHBoxLayout;

    QLabel* volumeS = new QLabel(tr("Volume"));
    QSlider* volumeSliderS = new QSlider(Qt::Horizontal);
    volumeSliderS->setValue(99);

    volumeSpeakersHBox->addWidget(volumeS);
    volumeSpeakersHBox->addWidget(volumeSliderS);
    volumeSpeakersHBox->addStretch();

    settings.beginGroup("AudioSettings");
    comboMicrophone->setCurrentText(settings.value("Microphone", comboMicrophone->itemText(0)).toString());
    volumeSliderM->setValue(settings.value("VolumeMicrophone", 99).toInt());
    comboSpeakers->setCurrentText(settings.value("Speakers", comboSpeakers->itemText(0)).toString());
    volumeSliderS->setValue(settings.value("VolumeSpeakers", 99).toInt());
    settings.endGroup();

    audioVBox->addWidget(infoAudioLabel);
    audioVBox->addSpacing(20);
    audioVBox->addLayout(microphoneHBox);
    audioVBox->addSpacing(20);
    audioVBox->addLayout(volumeMicrophoneHBox);
    audioVBox->addSpacing(20);
    audioVBox->addLayout(speakersHBox);
    audioVBox->addSpacing(20);
    audioVBox->addLayout(volumeSpeakersHBox);
    audioVBox->addStretch();

    audioWidget->setLayout(audioVBox);

    QWidget* videoWidget = new QWidget;
    QVBoxLayout* videoVBox = new QVBoxLayout;

    QLabel* infoVideoLabel = new QLabel(tr("Video setting: Webcam setting"));

    QHBoxLayout* cameraHBox = new QHBoxLayout;

    QLabel* cameraL = new QLabel(tr("Camera"));
    QComboBox* comboCamera= new QComboBox;
    comboCamera->setFixedWidth(200);

    foreach (const QCameraInfo &deviceInfo, QCameraInfo::availableCameras())
    {
        comboCamera->addItem(deviceInfo.description());
    }

    comboCamera->setCurrentIndex(-1);

    cameraHBox->addWidget(cameraL);
    cameraHBox->addWidget(comboCamera);
    cameraHBox->addStretch();

    QHBoxLayout* videoHBox = new QHBoxLayout;

    QLabel* video = new QLabel(tr("Video"));
    QCameraViewfinder* viewfinder = new QCameraViewfinder;
    viewfinder->setMaximumSize(250, 250);
    QCamera* camera = nullptr;
    if(QCameraInfo::availableCameras().count() > 0)
    {
        connect(comboCamera, &QComboBox::currentTextChanged, [&camera, viewfinder](const QString& description){
            if(camera != nullptr)
            {
                camera->deleteLater();
            }
            foreach (const QCameraInfo &deviceInfo, QCameraInfo::availableCameras())
            {
                if(deviceInfo.description() == description)
                {
                    camera = new QCamera(deviceInfo);
                }
            }
            camera->setViewfinder(viewfinder);
            camera->setCaptureMode(QCamera::CaptureStillImage);
            camera->start();
        });

        settings.beginGroup("VideoSettings");
        comboCamera->setCurrentText(settings.value("Camera", QCameraInfo::defaultCamera().description()).toString());
        settings.endGroup();
    }


    videoHBox->addWidget(video);
    videoHBox->addWidget(viewfinder);
    videoHBox->addStretch();

    videoVBox->addWidget(infoVideoLabel);
    videoVBox->addSpacing(20);
    videoVBox->addLayout(cameraHBox);
    videoVBox->addSpacing(20);
    videoVBox->addLayout(videoHBox);
    videoVBox->addStretch();

    videoWidget->setLayout(videoVBox);

    TabWidget *settingsTabWidget = new TabWidget;
    settingsTabWidget->setTabPosition(QTabWidget::West);
    settingsTabWidget->addTab(generalWidget, tr("General Settings"));
    settingsTabWidget->addTab(audioWidget, tr("Audio Settings"));
    settingsTabWidget->addTab(videoWidget, tr("Video Settings"));
    settingsTabWidget->addTab(new QWidget(this), tr("Privacy"));
    settingsTabWidget->addTab(new QWidget(this), tr("Notifications"));
    settingsTabWidget->addTab(new QWidget(this), tr("Advanced Settings"));

    QHBoxLayout* buttonsHBox = new QHBoxLayout;

    QPushButton* okButton = new QPushButton;
    okButton->setText(tr("Accept"));
    QString qss = ("QPushButton{"
                              "font-weight: 700;"
                              "text-decoration: none;"
                              "padding: .5em 2em;"
                              "outline: none;"
                              "border: 2px solid;"
                              "border-radius: 1px;"
                            "} "
                            "QPushButton:!hover { background: rgb(255,255,255); }");
    okButton->setStyleSheet(qss);

    // сохраняем все настройки в конигурационный файл и закрываем окно
    connect(okButton, &QPushButton::clicked, [comboLanguage, comboMicrophone, comboSpeakers,
            volumeSliderM, volumeSliderS, comboCamera, this](){
        QSettings settings("settings.conf", QSettings::IniFormat);

        settings.beginGroup("GeneralSettings");
        settings.setValue("Language", comboLanguage->currentText());
        settings.endGroup();

        settings.beginGroup("AudioSettings");
        settings.setValue("Microphone", comboMicrophone->currentText());
        settings.setValue("VolumeMicrophone", volumeSliderM->value());
        settings.setValue("Speakers", comboSpeakers->currentText());
        settings.setValue("VolumeSpeakers", volumeSliderS->value());
        settings.endGroup();

        settings.beginGroup("VideoSettings");
        settings.setValue("Camera", comboCamera->currentText());
        settings.endGroup();

        emit(updateSettings());

        QMessageBox::information(NULL,tr("Successfully"), tr("Settings successfully saved!"));

        deleteLater();
    });

    buttonsHBox->addStretch();
    buttonsHBox->addWidget(okButton);

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->addWidget(settingsTabWidget);
    mainVBox->addLayout(buttonsHBox);
    setLayout(mainVBox);
}
