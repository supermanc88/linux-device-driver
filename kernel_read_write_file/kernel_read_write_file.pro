TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += MODULE, __KERNEL__, KBUILD_MODNAME

QMAKE_CXXFLAGS += -IE:\\LinuxKernel\\linux-3.16.82\\include
QMAKE_CFLAGS += -IE:\\LinuxKernel\\linux-3.16.82\\include

SOURCES += \
    kern_rw_file.c

DISTFILES += \
    Makefile
