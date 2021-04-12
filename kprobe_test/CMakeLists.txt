cmake_minimum_required(VERSION 3.5)
project(kprobe_test C)

set(CMAKE_C_STANDARD 99)

set(INC_DIR /usr/src/linux-headers-4.4.0-21/include
        /usr/src/linux-headers-4.4.0-21/arch/arm64/include
        /usr/src/linux-headers-4.4.0-21/arch/arm/include)

include_directories(
        #E:\\LinuxKernel\\linux-5.4.50\\include
        #                    E:\\LinuxKernel\\linux-5.4.50\\include\\uapi
        #                    E:\\LinuxKernel\\linux-5.4.50\\include\\arch\\x86\\include
        #                    E:\\LinuxKernel\\linux-5.4.50\\include\\arch\\x86\\include\\uapi
        ${INC_DIR}
)

add_definitions(-DMODULE -D__KERNEL__ -D__GNUC__ -DCONFIG_64BIT -DCONFIG_KPROBES)

add_executable(kprobe_test kprobetest.c hookfunc.c common.h recordkey.c)