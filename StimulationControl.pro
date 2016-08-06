#-------------------------------------------------
#
# Project created by QtCreator 2016-06-30T14:43:51
#
#-------------------------------------------------

QT       += core gui serialport


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = StimulationControl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    stimulator.cpp \
    qcustomplot.cpp \
    slidemodel.cpp

HEADERS  += mainwindow.h \
    stimulator.h \
    qcustomplot.h \
    slidemodel.h

FORMS    += mainwindow.ui

win32: LIBS += -L$$PWD/./ -lOnLineInterface
