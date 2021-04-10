#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>

#define FUNC_NAME           "input_handle_event"
#define FUNC_RELOCATE       0
#define PRINT_LEN           40

static int __init get_signature_code_init(void)
{
    printk("%s\n", __func__);

    unsigned long func_addr = kallsyms_lookup_name(FUNC_NAME);
    printk("%s [%s] addr = [0x%lx]\n", __func__, FUNC_NAME, func_addr);

    int i = 0;
    unsigned char * opcode = (unsigned char *)func_addr;

    opcode += FUNC_RELOCATE;
    printk("%s printk start\n", __func__);
    for (i = 0; i < PRINT_LEN; i++) {
        printk("0x%02x\n", opcode[i]);
    }
    printk("%s printk end\n", __func__);

    return 0;
}


static void __exit get_signature_code_exit(void)
{
    printk("%s\n", __func__);
}


module_init(get_signature_code_init)

module_exit(get_signature_code_exit)

MODULE_LICENSE("GPL");