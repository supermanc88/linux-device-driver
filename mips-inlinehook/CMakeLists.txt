cmake_minimum_required(VERSION 3.13)
project(mips_inlinehook C)

set(CMAKE_C_STANDARD 99)

set(INC_DIR /usr/src/linux-headers-4.19.0-loongson-3-server/include
        /usr/src/linux-headers-4.19.0-loongson-3-server/arch/mips/include)

include_directories(
        ${INC_DIR}
)

add_definitions(-DMODULE -D__KERNEL__ -D__GNUC__ -DCONFIG_64BIT -DCONFIG_KPROBES)

add_subdirectory(userland)

add_executable(mips_inlinehook mips-inlinehook.c recordkey.c hookfunc.c)