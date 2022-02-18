#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x4e79ba4a, "module_layout" },
	{ 0x4b6dbab9, "cdev_del" },
	{ 0x7c8cd159, "kmalloc_caches" },
	{ 0xcaf4ced0, "cdev_init" },
	{ 0xac217106, "pci_write_config_word" },
	{ 0xd5b646c3, "pci_disable_device" },
	{ 0x6bd0e573, "down_interruptible" },
	{ 0xe2727a5b, "device_destroy" },
	{ 0x3a099605, "__get_user_nocheck_4" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0xc5850110, "printk" },
	{ 0xc9d795a0, "class_unregister" },
	{ 0x83703e5c, "pci_read_config_word" },
	{ 0x6626afca, "down" },
	{ 0xfa3af9f6, "device_create" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x2213bad9, "pci_enable_msi" },
	{ 0x2c7b3e8c, "cdev_add" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x39b11568, "pci_unregister_driver" },
	{ 0xa3670410, "kmem_cache_alloc_trace" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0xbb58adb3, "pci_clear_mwi" },
	{ 0x37a0cba, "kfree" },
	{ 0xb7a73ee0, "pci_disable_msi" },
	{ 0xedc03953, "iounmap" },
	{ 0xcf2a6966, "up" },
	{ 0x556422b3, "ioremap_cache" },
	{ 0xddc461a9, "__pci_register_driver" },
	{ 0x3422c712, "class_destroy" },
	{ 0x4a453f53, "iowrite32" },
	{ 0xd92da600, "pci_enable_device" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x41c03874, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xa78af5f3, "ioread32" },
	{ 0xc1514a3b, "free_irq" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010EEd00000070sv*sd*bc*sc*i*");
