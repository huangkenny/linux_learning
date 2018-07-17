#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
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
	{ 0x80381807, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xc6243ec9, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0xb5a459dc, __VMLINUX_SYMBOL_STR(unregister_blkdev) },
	{ 0x1452b512, __VMLINUX_SYMBOL_STR(put_disk) },
	{ 0xe1537255, __VMLINUX_SYMBOL_STR(__list_del_entry_valid) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xc2a1b680, __VMLINUX_SYMBOL_STR(blk_cleanup_queue) },
	{ 0x68f31cbd, __VMLINUX_SYMBOL_STR(__list_add_valid) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xacea2571, __VMLINUX_SYMBOL_STR(alloc_disk) },
	{ 0xcea93c1e, __VMLINUX_SYMBOL_STR(blk_queue_max_discard_sectors) },
	{ 0xdc892b39, __VMLINUX_SYMBOL_STR(blk_queue_physical_block_size) },
	{ 0xc940d523, __VMLINUX_SYMBOL_STR(blk_queue_bounce_limit) },
	{ 0xdfec4771, __VMLINUX_SYMBOL_STR(blk_queue_max_hw_sectors) },
	{ 0xbe2b18ee, __VMLINUX_SYMBOL_STR(blk_queue_make_request) },
	{ 0x369c4a1a, __VMLINUX_SYMBOL_STR(blk_alloc_queue) },
	{ 0x7c925456, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x92f02239, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x71a50dbc, __VMLINUX_SYMBOL_STR(register_blkdev) },
	{ 0xf2ee078, __VMLINUX_SYMBOL_STR(bio_endio) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x6a9d435e, __VMLINUX_SYMBOL_STR(kill_bdev) },
	{ 0xbf63e976, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x74aed678, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xba53db24, __VMLINUX_SYMBOL_STR(__free_pages) },
	{ 0x7e880422, __VMLINUX_SYMBOL_STR(radix_tree_delete) },
	{ 0x6e6b49d3, __VMLINUX_SYMBOL_STR(radix_tree_gang_lookup) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "B60897A8E391D525BF31A33");
