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
	../../../Timbre \
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
    ../../../Timbre/queue.c \
    mocks.cpp \
    testhandlers.cpp \
    testreceiverstatemachine.cpp \
    testtransmitterstatemachine.cpp \
    testsps.cpp \
    smallNetwork.cpp \
    testtalk.cpp \
    ../../../Timbre/printersHost.c

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
    testhandlers.h \
    testreceiverstatemachine.h \
    testtransmitterstatemachine.h \
    testsps.h \
    parameters.h \
    smallNetwork.h \
    testtalk.h
