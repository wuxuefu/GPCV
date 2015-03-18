#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc01e165c, "module_layout" },
	{ 0xb5e1ecd7, "platform_driver_register" },
	{ 0x9cde24cd, "platform_device_register" },
	{ 0xc633495b, "schedule_work" },
	{ 0x3e671187, "input_event" },
	{ 0x1c7423d3, "gp_gpio_get_value" },
	{ 0x7d11c268, "jiffies" },
	{ 0x7df0a1ba, "input_free_device" },
	{ 0x1bc672a3, "input_register_device" },
	{ 0x2dd77c66, "mod_timer" },
	{ 0xfcce18e3, "input_set_capability" },
	{ 0xd074a5c2, "gp_gpio_set_input" },
	{ 0xea147363, "printk" },
	{ 0xe5b1a67c, "gp_gpio_request" },
	{ 0xf229e057, "init_timer_key" },
	{ 0x2f494f9e, "dev_set_drvdata" },
	{ 0x91a18962, "input_allocate_device" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x37a0cba, "kfree" },
	{ 0xcb34d176, "input_unregister_device" },
	{ 0xc33e3e20, "gp_gpio_release" },
	{ 0x23869dc7, "cancel_work_sync" },
	{ 0xb6867ec3, "del_timer" },
	{ 0x3bad907b, "dev_get_drvdata" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x3030ea7f, "platform_driver_unregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "6B573999887E4BA737C8A10");
