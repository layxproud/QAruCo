QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    res.qrc

# OPENCV
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/third_party/opencv_mingw810/x64/mingw/bin/ -llibopencv_world4100
else:unix: LIBS += -L$$PWD/third_party/opencv_mingw810/x64/mingw/lib/ -llibopencv_world4100

INCLUDEPATH += $$PWD/third_party/opencv_mingw810/include
DEPENDPATH += $$PWD/third_party/opencv_mingw810/include

# AruCoAPI
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/third_party/AruCoAPI/ -lAruCoAPI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/third_party/AruCoAPI/ -lAruCoAPId
else:unix: LIBS += -L$$PWD/third_party/AruCoAPI/ -lAruCoAPI

INCLUDEPATH += $$PWD/third_party/AruCoAPI/include
DEPENDPATH += $$PWD/third_party/AruCoAPI/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
