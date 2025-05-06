#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/fs_context.h>

static struct fs_context_operations etcfs_context_ops = {
};

static int etcfs_init_fs_context(struct fs_context *fc) {
	int err;
	fc->ops = &etcfs_context_ops;
	return 0;
}

static struct file_system_type etc_fs_type = {
	.name = "etcfs",
	.f_flasgs = FS_USERNS_MOUNT,
	.init_fs_context = etcfs_init_fs_context,
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
	int err;
	err = unregister_filesystem(etcfs);
	if (err) {
		return err;
	}
	return 0;
}

module_init(etcfs_init);
module_exit(etcfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("c4llv07e <igor@c4llv07e.xyz>");
MODULE_DESCRIPTION("Read-only preconfigured etc filesystem");
