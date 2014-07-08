TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    SerialComms/serial.cc

HEADERS += \
    SerialComms/serial.h


QMAKE_CXXFLAGS += -fpermissive
