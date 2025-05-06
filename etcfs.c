#define DEBUG
#define ETCFS_MAGIC 0x49c643f4 /* random number */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/dcache.h>
#include <linux/mnt_idmapping.h>

#if 0
static const struct super_operations etcfs_ops = {

};
#endif

static int etcfs_fill_super(struct super_block *sb, strcut fc_context *fc) {
	struct inode *inode;
	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = ETCFS_MAGIC;
	sb->s_time_gran = 1; /* I have no idea what is's doing */

	inode = new_inode(sb);

	if (!inode)
		return -ENOMEM;

	inode->i_ino = get_next_ino();
	inode_init_owner(&nop_mnt_idmap, inode, NULL, S_IFDIR);
	inode->i_mapping->a_ops = &ram_aops;
	/*
	mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
	mapping_set_unevictable(inode->i_mapping);
	*/
	simple_inode_init_ts(inode);

	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;
	return 0;
}

static int etcfs_get_tree(struct fs_context *fc) {
	pr_debug("Get tree\n");
	return get_tree_nodev(fc, etcfs_fill_super);
}

static struct fs_context_operations etcfs_context_ops = {
	.get_tree = etcfs_get_tree,
};

static int etcfs_init_fs_context(struct fs_context *fc) {
	pr_debug("Etcfs init fs context\n");
	fc->ops = &etcfs_context_ops;
	return 0;
}

static struct file_system_type etcfs_fs_type = {
	.name = "etcfs",
	.fs_flags = FS_USERNS_MOUNT,
	.init_fs_context = etcfs_init_fs_context,
};

static int __init etcfs_init(void) {
	int err;
	err = register_filesystem(&etcfs_fs_type);
	if (err) {
		return err;
	}
	return 0;
}

static void __exit etcfs_exit(void) {
	int err;
	err = unregister_filesystem(&etcfs_fs_type);
	if (err) {
		pr_err("Error, can't unregister filesystem: %d\n", err);
		return;
	}
	return;
}

module_init(etcfs_init);
module_exit(etcfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("c4llv07e <igor@c4llv07e.xyz>");
MODULE_DESCRIPTION("Read-only preconfigured etc filesystem");
