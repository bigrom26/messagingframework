TEMPLATE = lib
TARGET = qmfsettings
CONFIG += qmfclient qmfmessageserver plugin

equals(QT_MAJOR_VERSION, 4): target.path += $$QMF_INSTALL_ROOT/lib/qmf/plugins/messageservices
equals(QT_MAJOR_VERSION, 5): target.path += $$QMF_INSTALL_ROOT/lib/qmf/plugins5/messageservices

QT = core network

DEPENDPATH += .

INCLUDEPATH += . ../../../libraries/qmfclient \
               ../../../libraries/qmfmessageserver \
               ../../../libraries/qmfclient/support

LIBS += -L../../../libraries/qmfclient/build \
        -L../../../libraries/qmfmessageserver/build
macx:LIBS += -F../../../libraries/qmfclient/build \
        -F../../../libraries/qmfmessageserver/build

HEADERS += service.h

SOURCES += service.cpp

!contains(DEFINES,QMF_NO_MESSAGE_SERVICE_EDITOR) {
    QT += gui
    equals(QT_MAJOR_VERSION, 5): QT += widgets

HEADERS += settings.h

FORMS += settings.ui

SOURCES += settings.cpp storagelocations.cpp
}

include(../../../../common.pri)
