QT += widgets

CONFIG += debug

LIBS += -lssl -lcrypto

HEADERS += \
    Headers/MainWindow.h \
    Headers/CC.h \
    Headers/RSA.h \

SOURCES += \
    Sources/MainWindow.cpp \
    Sources/CC.cpp \
    Sources/RSA.cpp \
    main.cpp