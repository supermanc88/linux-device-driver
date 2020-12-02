TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += MODULE, __KERNEL__, KBUILD_MODNAME

QMAKE_CXXFLAGS += -IE:\\LinuxKernel\\linux-4.0\\include
QMAKE_CFLAGS += -IE:\\LinuxKernel\\linux-4.0\\include

SOURCES += \
    kern_time.c

DISTFILES += \
    Makefile
