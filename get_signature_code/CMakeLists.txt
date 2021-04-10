cmake_minimum_required(VERSION 3.13)
project(get_signature_code C)

set(CMAKE_C_STANDARD 99)

set(INC_DIR /usr/src/linux-headers-4.19.0-loongson-3-server/include
        /usr/src/linux-headers-4.19.0-loongson-3-server/arch/mips/include)

include_directories(
        ${INC_DIR}
)

add_definitions(-DMODULE -D__KERNEL__ -D__GNUC__ -DCONFIG_64BIT -DCONFIG_KPROBES)

add_executable(get_signature_code get-signature-code.c)