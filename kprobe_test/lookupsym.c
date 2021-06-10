#include <linux/kallsyms.h>

#include "common.h"

/**
 * @brief 通过 kallsyms_lookup_name 函数查找指定函数名的地址，
 *        前提是 kallsyms_lookup_name 函数被内核导出。
 * @param sym_name
 * @return
 */
unsigned long lookup_symbol_by_name(const char * sym_name)
{
    printk("%s sym_name = [%s]\n", __func__, sym_name);
    return kallsyms_lookup_name(sym_name);
}