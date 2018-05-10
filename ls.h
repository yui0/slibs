/* public domain Simple, Minimalistic, making list of files and directories
 *	©2017 Yuichiro Nakada
 *
 * Basic usage:
 *	int num;
 *	LS_LIST *ls = ls_dir("dir/", LS_RECURSIVE|LS_RANDOM, &num);
 * */

#include <dirent.h>
#include <sys/stat.h>

#define LS_RECURSIVE	1
#define LS_RANDOM	2

typedef struct {
	int status;
	char d_name[PATH_MAX];
} LS_LIST;

int ls_count_dir(char *dir, int flag)
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

		if (flag & LS_RECURSIVE) {
			stat(entry->d_name, &statbuf);
			if (S_ISDIR(statbuf.st_mode)) {
				char path[PATH_MAX];
				sprintf(path, "%s/%s", dir, entry->d_name);
				i += ls_count_dir(path, flag);
			}
		}

		i++;
	}

	closedir(dp);
	chdir(cpath);
	free(cpath);

	return i;
}
	
int ls_seek_dir(char *dir, LS_LIST *ls, int flag)
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

			//if (flag & LS_RECURSIVE) i += ls_seek_dir(buf, ls+i+1, flag);
			if (flag & LS_RECURSIVE) i += ls_seek_dir(buf, ls+i, flag);
		} else {
			(ls+i)->status = 0;
			i++;
		}
		//i++;
	}

	closedir(dp);
	chdir(cpath);
	free(cpath);

	return i;
}

int ls_comp_func(const void *a, const void *b)
{
	return (strcmp((char*)(((LS_LIST*)a)->d_name), (char*)(((LS_LIST*)b)->d_name)));
}

LS_LIST *ls_dir(char *dir, int flag, int *num)
{
	int n = ls_count_dir(dir, flag);
	if (!n) {
		fprintf(stderr, "No file found [%s]!!\n", dir);
		return 0;
	}

	LS_LIST *ls = (LS_LIST *)calloc(n, sizeof(LS_LIST));
	if (!ls) {
		perror("calloc");
		return 0;
	}

	if (!ls_seek_dir(dir, ls, flag)) return 0;

	if (flag & LS_RANDOM) {
#ifdef RANDOM_H
		xor128_init(time(NULL));
#else
		srand(time(NULL));
#endif
		for (int i=0; i<n; i++) {
#ifdef RANDOM_H
			int a = frand() * n;
#else
			int a = rand()%n;
#endif
			LS_LIST b = ls[i];
			ls[i] = ls[a];
			ls[a] = b;
		}
	} else {
		qsort(ls, n, sizeof(LS_LIST), ls_comp_func);
	}

	*num = n;
	return ls;
}

#include <ctype.h>
char *findExt(char *path)
{
	static char ext[10];
	char *e = &ext[9];
	*e-- = 0;
	int len = strlen(path)-1;
	for (int i=len; i>len-9; i--) {
		if (path[i] == '.' ) break;
		*e-- = tolower(path[i]);
	}
	return e+1;
}
