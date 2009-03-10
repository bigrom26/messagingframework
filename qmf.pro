TEMPLATE = subdirs
SUBDIRS = src/libraries/qtopiamail \
          src/libraries/messageserver \
          src/libraries/qmfutil \
          src/plugins/messageservices/imap \
          src/plugins/messageservices/pop \
          src/plugins/messageservices/smtp \
          src/plugins/messageservices/qtopiamailfile \
          src/plugins/contentmanagers/qtopiamailfile \
          src/plugins/viewers/generic \
          src/plugins/composers/email \
          src/settings/messagingaccounts \
          src/tools/messageserver \
          src/applications/qtmail \
          tests \
          benchmarks \

CONFIG += ordered

# Make it so projects can find our specific features
system(echo QMAKEFEATURES+=$$PWD/features > $$OUT_PWD/.qmake.cache)

