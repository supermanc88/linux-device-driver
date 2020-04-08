TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += MODULE, __KERNEL__, KBUILD_MODNAME, CONFIG_X86_64

QMAKE_CXXFLAGS += -IE:\\LinuxKernel\\linux-2.6.32.69\\include -IE:\LinuxKernel\linux-2.6.32.69\arch\x86\include
QMAKE_CFLAGS += -IE:\\LinuxKernel\\linux-2.6.32.69\\include -IE:\LinuxKernel\linux-2.6.32.69\arch\x86\include

SOURCES += \
    linux_syscall_hook.c
