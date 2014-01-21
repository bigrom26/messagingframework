TEMPLATE = lib 
TARGET = genericviewer 
CONFIG += plugin

target.path += $$QMF_INSTALL_ROOT/lib/qmf/plugins5/viewers
QT += widgets qmfclient
macx:contains(QT_CONFIG, qt_framework) {
    LIBS += -framework qmfutil5
} else {
    LIBS += -lqmfutil5
}

# Use webkit to render mail if available
contains(QT_CONFIG,webkit){
    QT += network webkitwidgets
    DEFINES += USE_WEBKIT
}

DEPENDPATH += .

INCLUDEPATH += . ../../../libs/qmfutil

LIBS += -L../../../libs/qmfutil/build
LIBS += -F../../../libs/qmfutil/build

HEADERS += attachmentoptions.h browserwidget.h genericviewer.h

SOURCES += attachmentoptions.cpp browserwidget.cpp genericviewer.cpp

TRANSLATIONS += libgenericviewer-ar.ts \
                libgenericviewer-de.ts \
                libgenericviewer-en_GB.ts \
                libgenericviewer-en_SU.ts \
                libgenericviewer-en_US.ts \
                libgenericviewer-es.ts \
                libgenericviewer-fr.ts \
                libgenericviewer-it.ts \
                libgenericviewer-ja.ts \
                libgenericviewer-ko.ts \
                libgenericviewer-pt_BR.ts \
                libgenericviewer-zh_CN.ts \
                libgenericviewer-zh_TW.ts

include(../../../../../common.pri)
