#ifndef ETCFS_CONFIG_INC
#define ETCFS_CONFIG_INC

#include "etcfs.h"

static ssize_t const_content(struct file *file, char __user *buf,
		size_t len, loff_t *offset, void *data) {
	const char *content = (const char *)data;
	size_t content_len = strlen(content);
	if (*offset >= content_len) return 0;
	if (len > content_len - *offset) len = content_len - *offset;
	if (copy_to_user(buf, content + *offset, len)) return -EFAULT;
	*offset += len;
	return len;
}

static int some_val;

static ssize_t print_some_val(struct file *file, char __user *buf,
		size_t len, loff_t *offset, void *data) {
	char result[0x10];
	size_t result_len = snprintf(result, sizeof result, "%d", some_val);
	if (*offset >= result_len) return 0;
	if (len > result_len - *offset) len = result_len - *offset;
	if (copy_to_user(buf, result + *offset, len)) return -EFAULT;
	*offset += len;
	if (*offset >= result_len)
		some_val += 1;
	return len;
}

struct etcfs_file_elem etcfs_files[] = {
	{"a", const_content, "config file"},
	{"b", print_some_val, NULL},
};
#endif /* ETCFS_CONFIG_INC */
