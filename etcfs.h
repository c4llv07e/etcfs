#ifndef ETCFS_HEADER_INC
#define ETCFS_HEADER_INC

#define LEN(x) ((sizeof (x)) / (sizeof (*(x))))

struct etcfs_file_elem {
	char *path;
	ssize_t (*read)(struct file *file, char __user *buf,
		size_t len, loff_t *offset, void *data);
	void *data;
};

#endif /* ETCFS_HEADER_INC */
