QT -= gui
QT += testlib

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_server_library

INCLUDEPATH += $$PWD/../server_library

SOURCES += \
    tst_generalutilities.cpp

LIBS += -L$$OUT_PWD/../bin -lserver_library
DESTDIR = ../bin
