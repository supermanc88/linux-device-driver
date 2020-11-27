cmake_minimum_required(VERSION 3.5)

INCLUDE_DIRECTORIES(E:/LinuxKernel/linux-3.16.82/include)

add_definitions(
        -DMODULE
        -D__KERNEL__
        -D__GNUC__
)

project(get_sys_call_table LANGUAGES C)

add_executable(get_sys_call_table get_sct.c)
