#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_MITIGATION_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xc983fe3, "proc_create" },
	{ 0xdcb764ad, "memset" },
	{ 0x122c3a7e, "_printk" },
	{ 0x472cf3b, "register_kprobe" },
	{ 0x2fce0754, "remove_proc_entry" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x1b76994, "single_open" },
	{ 0xbbe44e86, "seq_printf" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0xeb78b1ed, "unregister_kprobe" },
	{ 0x7da992cc, "seq_read" },
	{ 0x64d9503, "seq_lseek" },
	{ 0x3958da9c, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "CF3D275C7E0219D3F8EB758");
