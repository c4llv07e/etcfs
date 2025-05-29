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
static struct dentry *etcfs_lookup(struct inode *dir, struct dentry *dentry,
				unsigned int flags) {
	return dentry->d_parent;
}

static const struct inode_operations etcfs_dir_inode_operations = {
	.lookup = etcfs_lookup,
};

static int etcfs_readdir(struct file *file, struct dir_context *ctx) {
	if (!dir_emit_dots(file, ctx))
		return 0;
	if (!dir_emit(ctx, "a", 1, 2, 0))
		return 0;
	return 0;
}

static struct file_operations etcfs_dir_file_ops = {
	.llseek = generic_file_llseek,
	.read = generic_read_dir,
	.iterate_shared = etcfs_readdir,
};
#endif

static struct inode *etcfs_create_inode(struct super_block *sb, int mode) {
	struct inode *inode;
	inode = new_inode(sb);

	if (!inode)
		return NULL;

	inode->i_ino = get_next_ino();
	inode->i_sb = sb;
	simple_inode_init_ts(inode);
	inode_init_owner(&nop_mnt_idmap, inode, NULL, S_IFDIR);
	return inode;
}

static int etcfs_fill_super(struct super_block *sb, struct fs_context *fs) {
	struct inode *inode;
	struct dentry *root;
	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = ETCFS_MAGIC;
	sb->s_time_gran = 1; /* I have no idea what is's doing */

	inode = etcfs_create_inode(sb, S_IFDIR | 0555);
	if (!inode)
		return -ENOMEM;

	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = &simple_dir_operations;

	root = d_make_root(inode);
	if (!root)
		return -ENOMEM;
	
	sb->s_root = root;
	return 0;
}

static int etcfs_get_tree(struct fs_context *fc) {
	return get_tree_nodev(fc, etcfs_fill_super);
}

static void etcfs_kill_sb(struct super_block *sb) {
	kill_litter_super(sb);
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
	.owner = THIS_MODULE,
	.name = "etcfs",
	.kill_sb = etcfs_kill_sb,
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
