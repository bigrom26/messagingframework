TEMPLATE = app
target.path += $$QMF_INSTALL_ROOT/bin
TARGET = messagingaccounts5
QT += widgets qmfclient qmfmessageserver

QTMAIL_EXAMPLE=../qtmail

#Required to build on windows
DEFINES += QMFUTIL_INTERNAL

INCLUDEPATH += \
                 $$QTMAIL_EXAMPLE/app \
                 $$QTMAIL_EXAMPLE/libs/qmfutil

HEADERS += $$QTMAIL_EXAMPLE/app/accountsettings.h \
           $$QTMAIL_EXAMPLE/app/editaccount.h \
           $$QTMAIL_EXAMPLE/app/statusbar.h \
           $$QTMAIL_EXAMPLE/libs/qmfutil/qtmailnamespace.h

SOURCES += $$QTMAIL_EXAMPLE/app/accountsettings.cpp \
           $$QTMAIL_EXAMPLE/app/editaccount.cpp \
           main_messagingaccounts.cpp \
           $$QTMAIL_EXAMPLE/app/statusbar.cpp \
           $$QTMAIL_EXAMPLE/libs/qmfutil/qtmailnamespace.cpp

RESOURCES += messagingaccounts.qrc

include(../../common.pri)
