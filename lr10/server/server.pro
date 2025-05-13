QT += core network widgets
CONFIG += c++17 cmdline

SOURCES += \
    main.cpp \
    server.cpp

HEADERS += \
    server.h

# Deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
