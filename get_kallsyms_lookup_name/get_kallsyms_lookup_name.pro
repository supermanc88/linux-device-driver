TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += MODULE, __KERNEL__, KBUILD_MODNAME, __GNUC__

QMAKE_CXXFLAGS += -IE:\\LinuxKernel\\linux-2.6.32-754.el6\\include -IE:\\LinuxKernel\\linux-2.6.32-754.el6\\include\\uapi
QMAKE_CFLAGS += -IE:\\LinuxKernel\\linux-2.6.32-754.el6\\include -IE:\\LinuxKernel\\linux-2.6.32-754.el6\\include\\uapi

SOURCES += \
    get_syms.c

DISTFILES += \
    Makefile
