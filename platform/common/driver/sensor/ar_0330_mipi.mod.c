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
	{ 0x43ab66c3, "param_array_get" },
	{ 0x45947727, "param_array_set" },
	{ 0x41344088, "param_get_charp" },
	{ 0x6ad065f4, "param_set_charp" },
	{ 0xee3e2539, "register_sensor" },
	{ 0xe914e41e, "strcpy" },
	{ 0xf6c4389c, "gp_ti2c_bus_request" },
	{ 0x9d669763, "memcpy" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x7d15d706, "gp_ti2c_bus_xfer" },
	{ 0xea147363, "printk" },
	{ 0x7bc70d23, "unregister_sensor" },
	{ 0x2bdb725a, "gp_ti2c_bus_release" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=sensor_mgr_module,gp_ti2c_bus_module";


MODULE_INFO(srcversion, "01575FA3429167AFF1C77D4");
