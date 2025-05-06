#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

static struct file_system_type etc_fs_type = {
	.name = "etcfs",
	.f_flasgs = FS_USERNS_MOUNT,
};

static int __init etcfs_init(void) {
	int err;
	err = register_filesystem(etcfs);
	if (err) {
		return err;
	}
	return 0;
}

static int __exit etcfs_exit(void) {
	return 0;
}

module_init(etcfs_init);
module_exit(etcfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("c4llv07e <igor@c4llv07e.xyz>");
MODULE_DESCRIPTION("Read-only preconfigured etc filesystem");
