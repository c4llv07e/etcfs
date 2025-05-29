#define DEBUG
#define ETCFS_MAGIC 0x49c643f4 /* random number */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/dcache.h>
#include <linux/mnt_idmapping.h>

#define LEN(x) ((sizeof (x)) / (sizeof (*(x))))

struct etcfs_file_elem {
	char *path;
	char *value;
};

struct etcfs_file_elem etcfs_files[] = {
	{"a", "config file"},
	{"b", "second file"},
};

static const char *etcfs_find_content(const char *path) {
	size_t i;
	for (i = 0; i < LEN(etcfs_files); ++i) {
		if (strcmp(etcfs_files[i].path, path) == 0) {
			return etcfs_files[i].value;
		}
	}
	return NULL;
}

static ssize_t etcfs_file_read(struct file *file, char __user *buf,
		size_t len, loff_t *offset) {
	const char *filepath = file->f_path.dentry->d_name.name;
	pr_debug("Reading file %s\n", filepath);
	const char *msg = etcfs_find_content(filepath);
	if (msg == NULL) return -EINVAL;
	size_t msg_len = strlen(msg);
	if (*offset >= msg_len) return 0;
	if (len > msg_len - *offset) len = msg_len - *offset;
	if (copy_to_user(buf, msg + *offset, len)) return -EFAULT;
	*offset += len;
	return len;
}

static struct file_operations etcfs_file_fops = {
	.owner = THIS_MODULE,
	.read = etcfs_file_read,
};

static struct inode *etcfs_create_inode(struct super_block *sb, int mode) {
	pr_debug("Creating inode\n");
	struct inode *inode;
	inode = new_inode(sb);

	if (!inode)
		return NULL;

	inode->i_ino = get_next_ino();
	inode->i_sb = sb;
	simple_inode_init_ts(inode);
	inode_init_owner(&nop_mnt_idmap, inode, NULL, mode);
	return inode;
}

static struct dentry *etcfs_lookup(struct inode *dir, struct dentry *dentry,
				unsigned int flags) {
	pr_debug("Looking up %s...\n", dentry->d_name.name);
	const char *filepath = dentry->d_name.name;
	if (etcfs_find_content(filepath) == NULL) return ERR_PTR(-ENOENT);
	struct inode *inode;
	inode = etcfs_create_inode(dir->i_sb, S_IFREG | 0444);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	inode->i_fop = &etcfs_file_fops;
	d_add(dentry, inode);
	return NULL;
}

static const struct inode_operations etcfs_dir_inode_operations = {
	.lookup = etcfs_lookup,
};

static int etcfs_readdir(struct file *file, struct dir_context *ctx) {
	pr_debug("Readding dir\n");
	if (!dir_emit_dots(file, ctx))
		return 0;
	if (ctx->pos == 2) {
		size_t n;
		for (n = 0; n < LEN(etcfs_files); ++n) {
			struct etcfs_file_elem efile = etcfs_files[n];
			dir_emit(ctx, efile.path, strlen(efile.path), 2, 0);
		}
		ctx->pos += LEN(etcfs_files);
	}
	return 0;
}

static struct file_operations etcfs_dir_file_ops = {
	.open = dcache_dir_open,
	.release = dcache_dir_close,
	.llseek = dcache_dir_lseek,
	.read = generic_read_dir,
	.iterate_shared = etcfs_readdir,
	.fsync = noop_fsync,
};

static int etcfs_fill_super(struct super_block *sb, struct fs_context *fs) {
	pr_debug("Filling super\n");
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

	inode->i_op = &etcfs_dir_inode_operations ;
	inode->i_fop = &etcfs_dir_file_ops;

	root = d_make_root(inode);
	if (!root)
		return -ENOMEM;
	
	sb->s_root = root;
	return 0;
}

static int etcfs_get_tree(struct fs_context *fc) {
	pr_debug("Getting tree\n");
	return get_tree_nodev(fc, etcfs_fill_super);
}

static void etcfs_kill_sb(struct super_block *sb) {
	pr_debug("Killing super\n");
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
