#-------------------------------------------------
#
# Project created by QtCreator 2015-02-19T17:54:08
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_sfpunitteststest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../ \
	../../../Timbre

SOURCES += \
    testframing.cpp \
    testservices.cpp \
    testrouting.cpp \
    testnetwork.cpp \
    testuart.cpp \
    testspi.cpp \
	testRunner.cpp \
    ../../frame.c \
    testpool.cpp \
    ../../../Timbre/queue.c \
    mocks.cpp \
    testhandlers.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    testframing.h \
    testservices.h \
    testrouting.h \
    testnetwork.h \
    testuart.h \
	testspi.h \
    ../../frame.h \
    ../../sfp.h \
    ../../../Timbre/bktypes.h \
    ../../../Timbre/byteq.h \
    ../../../Timbre/machines.h \
    ../../../Timbre/queue.h \
    ../../../Timbre/timbre.h \
    ../../../Timbre/timeout.h \
    testpool.h \
    mocks.h \
    testhandlers.h
