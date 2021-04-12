# 目录

- [get_kallsyms_lookup_name](./get_kallsyms_lookup_name)

  如果内核没有导出`kallsyms_lookup_name`，但导出了`kallsyms_on_each_symbol`函数，可通过此函数获取`kallsyms_lookup_name`的地址。

- [get_key_by_input_event_inlinehook](./get_key_by_input_event_inlinehook)

  通过inlinehook的方式hook`input_event`函数，来获取键盘按键

- [get_no_export_function](./get_no_export_function)

- [get_signature_code](./get_signature_code)

  用来打印指定地址指定长度的16进制数据，在不能调试内核，获取opcodes用的。

- [get_sys_call_table](./get_sys_call_table)

  获取sys_call_table

- [get_time](./get_time)

  在内核中获取当前时间

- [hook_profile_task_exit](./hook_profile_task_exit)

  hook进程退出函数

- [kernel_read_write_file](./kernel_read_write_file)

  内核中读取文件

- [kprobe_test](./kprobe_test)

  kprobe框架测试

- [linux_syscall_hook](./linux_syscall_hook)

  系统调用hook

- [list_process](./list_process)

  列出所有进程

- [mips-inlinehook](./mips-inlinehook)

  mips inlinehook测试

- [module_cdev](./module_cdev)

  字符设备模板

- [module_kthread](./module_kthread)

  内核中创建线程

- [module_template](./module_template)

  设备驱动模板

 