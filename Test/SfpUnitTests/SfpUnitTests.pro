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
QMAKE_MAC_SDK = macosx10.12

INCLUDEPATH += ../../ \
        ../../../TimbreOS \
	../../Handlers

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
    ../../../TimbreOS/queue.c \
    mocks.cpp \
    testhandlers.cpp \
    testreceiverstatemachine.cpp \
    testtransmitterstatemachine.cpp \
    testsps.cpp \
    smallNetwork.cpp \
    testtalk.cpp \
    ../../../TimbreOS/printersHost.c

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
    ../../../TimbreOS/ttypes.h \
    ../../../TimbreOS/byteq.h \
    ../../../TimbreOS/machines.h \
    ../../../TimbreOS/queue.h \
    ../../../TimbreOS/timbre.h \
    ../../../TimbreOS/timeout.h \
    testpool.h \
    mocks.h \
    testhandlers.h \
    testreceiverstatemachine.h \
    testtransmitterstatemachine.h \
    testsps.h \
    smallNetwork.h \
    testtalk.h
