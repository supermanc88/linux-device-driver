TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += MODULE, __KERNEL__, KBUILD_MODNAME

QMAKE_CXXFLAGS += -IE:\\LinuxKernel\\linux-2.6.32.69\\include
QMAKE_CFLAGS += -IE:\\LinuxKernel\\linux-2.6.32.69\\include

SOURCES += \
    module_cdev.c
