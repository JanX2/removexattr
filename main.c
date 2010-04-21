/* Compile by
 * gcc -O2 -Wall removexattr.c -o removexattr
 *
 * Run it on a file or a directory (it traverses down a directory)
 * Example: ./removexattr Downloads
 * will strip everything in the directory Downloads of the quarantine attribute.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/errno.h>
#include <sys/xattr.h>
#include <ctype.h>
#include <limits.h>


static void traverse_tree(const char * path);

/* You can add more attributes below if you want to */
static const char * const xattr_list[] = {
	"com.apple.quarantine",
	/* "com.apple.FinderInfo",*/
	NULL
};

static __inline__ void remove_xr(const char * path)
{
	int i = 0;
	
	for(i = 0; xattr_list[i]; i++) {
		if(getxattr(path, xattr_list[i], NULL, 0, 0, XATTR_NOFOLLOW) > 0)
			removexattr(path, xattr_list[i], XATTR_NOFOLLOW);
	}
}

int main(int argc, char ** argv)
{
	struct stat stbuf;
	int i;
	
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <file || directory ...>\n", argv[0]);
		exit(1);
	}
	
	for(i = 1; i < argc; i++) {
		if(stat(argv[i], &stbuf)) {
			perror(argv[i]);
			continue;
		}
		if(S_ISREG(stbuf.st_mode))
			remove_xr(argv[i]);
		else if(S_ISDIR(stbuf.st_mode))
			traverse_tree(argv[i]);
	}
	
	return(0);
}

#define BUFSIZE (PATH_MAX + 1)

static void traverse_tree(const char * path)
{
	DIR * dirp;
	struct dirent * dp;
	char * npath = NULL;
	
	if((dirp = opendir(path)) == NULL) {
		perror(path);
		return;
	}
	
	if(!(npath = malloc(BUFSIZE * sizeof(char)))) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	while((dp = readdir(dirp))) {
		if(!((dp->d_name[0] == '.' && dp->d_name[1] == '\0') ||
			 (dp->d_name[0] == '.' && dp->d_name[1] == '.' && dp->d_name[2] == '\0'))) {
			snprintf(npath,  BUFSIZE, "%s/%s", path, dp->d_name);
			remove_xr(npath);
			if(dp->d_type == DT_DIR) {
				traverse_tree(npath);
			}
		}
	}
	closedir(dirp);
	free(npath);
}
