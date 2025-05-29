#ifndef ETCFS_HEADER_INC
#define ETCFS_HEADER_INC

#define LEN(x) ((sizeof (x)) / (sizeof (*(x))))

struct etcfs_file_elem {
	char *path;
	char *value;
};

#endif /* ETCFS_HEADER_INC */
