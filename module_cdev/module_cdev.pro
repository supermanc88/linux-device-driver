TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += MODULE, __KERNEL__, KBUILD_MODNAME, __GNUC__

QMAKE_CXXFLAGS += -IE:\\LinuxKernel\\linux-3.16.82\\include -IE:\\LinuxKernel\\linux-3.16.82\\include\\uapi
QMAKE_CFLAGS += -IE:\\LinuxKernel\\linux-3.16.82\\include -IE:\\LinuxKernel\\linux-3.16.82\\include\\uapi

SOURCES += \
    module_cdev.c \
    user/cdev_test.c

DISTFILES += \
    Makefile
