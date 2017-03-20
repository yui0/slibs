/* public domain Simple, Minimalistic, making list of files and directories
 *	©2017 Yuichiro Nakada
 *
 * */

#include <dirent.h>
#include <sys/stat.h>

typedef struct {
	int status;
	char d_name[PATH_MAX];
} LS_LIST;

int count_dir(char *dir, int flag)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;

	if (!(dp = opendir(dir))) {
		perror("opendir");
		return 0;
	}
	char *cpath = getcwd(0, 0);
	chdir(dir);

	int i=0;
	while ((entry = readdir(dp))) {
		if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) continue;

		if (flag) {
			stat(entry->d_name, &statbuf);
			if (S_ISDIR(statbuf.st_mode)) {
				char path[PATH_MAX];
				sprintf(path, "%s/%s", dir, entry->d_name);
				i += count_dir(path, flag);
			}
		}

		i++;
	}

	closedir(dp);
	chdir(cpath);
	free(cpath);

	return i;
}
	
int seek_dir(char *dir, LS_LIST *ls, int flag)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char buf[PATH_MAX];

	if (!(dp = opendir(dir))) {
		perror("opendir");
		printf("%s\n", dir);
		return 0;
	}
	char *cpath = getcwd(0, 0);
	chdir(dir);

	int i=0;
	while ((entry = readdir(dp))) {
		if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) continue;

		sprintf(buf, "%s/%s", dir, entry->d_name);
		strcpy((ls+i)->d_name, buf);

		stat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			(ls+i)->status = 1;

			if (flag) i += seek_dir(buf, ls+i+1, flag);
		} else {
			(ls+i)->status = 0;
		}
		i++;
	}

	closedir(dp);
	chdir(cpath);
	free(cpath);

	return i;
}

int comp_func(const void *a, const void *b)
{
	return (strcmp((char*)(((LS_LIST*)a)->d_name), (char*)(((LS_LIST*)b)->d_name)));
}

LS_LIST *ls_dir(char *dir, int flag, int *num)
{
	int n = count_dir(dir, flag);
	if (!n) {
		fprintf(stderr, "No file found [%s]!!\n", dir);
		return 0;
	}

	LS_LIST *ls = (LS_LIST *)calloc(n, sizeof(LS_LIST));
	if (!ls) {
		perror("calloc");
		return 0;
	}

	if (!seek_dir(dir, ls, flag)) return 0;

	qsort(ls, n, sizeof(LS_LIST), comp_func);
	*num = n;
	return ls;
}

