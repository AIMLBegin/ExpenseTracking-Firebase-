QT += core gui network
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    authhandler.cpp

HEADERS += \
    authhandler.h

FORMS += \
    authhandler.ui

INCLUDEPATH += .
