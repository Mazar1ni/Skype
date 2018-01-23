#-------------------------------------------------
#
# Project created by QtCreator 2017-09-11T22:02:56
#
#-------------------------------------------------

QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Skype
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# ¬ыбираем директорию сборки исполн€емого файла
# в зависимости от режима сборки проекта
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/../../Skype/Debug
} else {
    DESTDIR = $$OUT_PWD/../../Skype/Release
}
# раздел€ем по директори€м все выходные файлы проекта
MOC_DIR = ../common/build/moc
RCC_DIR = ../common/build/rcc
UI_DIR = ../common/build/ui
unix:OBJECTS_DIR = ../common/build/o/unix
win32:OBJECTS_DIR = ../common/build/o/win32
macx:OBJECTS_DIR = ../common/build/o/mac

# в зависимости от режима сборки проекта
# запускаем win deploy приложени€ в целевой директории, то есть собираем все dll
CONFIG(debug, debug|release) {
    QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../Skype/Debug
} else {
    QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../Skype/Release
}


SOURCES += \
        main.cpp \
    UI/widget.cpp \
    UI/authentication.cpp \
    UI/settingsconnection.cpp \
    skype.cpp \
    mainwindow.cpp \
    friendwidget.cpp \
    calling.cpp \
    sox.cpp \
    message.cpp

HEADERS += \
    UI/widget.h \
    UI/authentication.h \
    UI/settingsconnection.h \
    skype.h \
    mainwindow.h \
    friendwidget.h \
    calling.h \
    sox.h \
    message.h

RESOURCES += \
    Resources/resources.qrc
