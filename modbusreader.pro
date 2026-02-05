QT       += core gui serialbus serialport widgets

# 注释掉charts，先确保基础功能能编译
# QT += charts

CONFIG   += c++11

# 如果使用MinGW，可能需要这个
# win32-g++: CONFIG += static

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    modbusclient.cpp

HEADERS += \
    mainwindow.h \
    modbusclient.h

# 对于Windows，可能需要指定库路径
win32 {
    # 如果你的Qt安装在非标准位置
    # INCLUDEPATH += "C:/Qt/5.15.2/mingw81_64/include"
    # LIBS += -L"C:/Qt/5.15.2/mingw81_64/lib" -lQt5Core -lQt5Gui -lQt5Widgets -lQt5SerialBus -lQt5SerialPort
}

#Release模式优化
CONFIG(release, debug|release): {
    DEFINES += QT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS_RELEASE += -O2
}

# Debug模式配置
# CONFIG(debug, debug|release): {
#     DEFINES += QT_DEBUG
# }


# 输出目录
DESTDIR = $$PWD../build/bin
OBJECTS_DIR = $$../build/PWD/obj
MOC_DIR = $$../build/PWD/moc
RCC_DIR = $$../build/PWD/rcc
UI_DIR = $$../build/PWD/ui

# # 默认规则
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target
