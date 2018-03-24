#include "settingswidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(700, 500);
    setWindowIcon(QIcon(":/Icons/skype_icon.ico"));

    QWidget* generalWidget = new QWidget;
    QVBoxLayout* generalVBox = new QVBoxLayout;

    QLabel* infoGeneralLabel = new QLabel("General settings: Basic settings for Skype");

    QHBoxLayout* languageHBox = new QHBoxLayout;

    QLabel* interfaceLanguage = new QLabel("Choice of interface language");
    QComboBox* comboLanguage = new QComboBox;
    comboLanguage->setFixedWidth(200);
    comboLanguage->addItem("English");
    comboLanguage->addItem("Russian");

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

    QLabel* infoAudioLabel = new QLabel("Sound setting: Set the sound on your computer");

    QHBoxLayout* microphoneHBox = new QHBoxLayout;

    QLabel* microphone = new QLabel("Microphone");
    QComboBox* comboMicrophone= new QComboBox;
    comboMicrophone->setFixedWidth(200);


    microphoneHBox->addWidget(microphone);
    microphoneHBox->addWidget(comboMicrophone);
    microphoneHBox->addStretch();

    audioVBox->addWidget(infoAudioLabel);
    audioVBox->addSpacing(20);
    audioVBox->addLayout(microphoneHBox);
    audioVBox->addStretch();

    audioWidget->setLayout(audioVBox);

    TabWidget *settingsTabWidget = new TabWidget;
    settingsTabWidget->setTabPosition(QTabWidget::West);
    settingsTabWidget->addTab(generalWidget, "General Settings");
    settingsTabWidget->addTab(new QWidget(this), "Audio Settings");
    settingsTabWidget->addTab(new QWidget(this), "Video Settings");
    settingsTabWidget->addTab(new QWidget(this), "Privacy");
    settingsTabWidget->addTab(new QWidget(this), "Notifications");
    settingsTabWidget->addTab(new QWidget(this), "Advanced");

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->addWidget(settingsTabWidget);
    setLayout(mainVBox);
}
