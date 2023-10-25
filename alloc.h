/* public domain Simple, Minimalistic, safety alloc
 *	©2023 Yuichiro Nakada
 *
 * Basic usage:
 *	only include this file...
 *
 * */

#include <stdlib.h>

#define MEM_ALIGN	256

#if defined(_MSC_VER) || defined(__MINGW32__)
#define _malloc(size)	_aligned_malloc(size, MEM_ALIGN)
#define _free(p)	_aligned_free(p)
#else
#define _malloc(size)	({ void* _p; posix_memalign((void**) &_p, MEM_ALIGN, (size))==0 ? _p : NULL; })
#define _free(p)	free(p)
#endif  /* _MSC_VER */
//#define calloc(n, size)	({ uint64_t _s = n * size; void* _p = malloc(_s); memset(_p, 0, _s)!=0 ? _p : NULL; })
//#define calloc(n, size)	({ uint64_t _s = n * size; void* _p; posix_memalign((void**) &_p, MEM_ALIGN, (_s))==0 ? _p : NULL; })

#define LEAK_MAX	500
typedef struct {
	void *ptr;
	size_t size;
	const char *file;
	unsigned int line;
} MEM_T;
MEM_T mem_info[LEAK_MAX];
static int leak_detect_initialized = 0;

static void leak_detect_free(void *ptr)
{
	for (int i=0; i<LEAK_MAX; i++) {
		if (mem_info[i].ptr == ptr) {
			mem_info[i].ptr = NULL;
			mem_info[i].size = 0;
			mem_info[i].file = NULL;
			mem_info[i].line = 0;
			break;
		}
	}
	_free(ptr);
}

static void leak_detect_check()
{
	for (int i=0; i<LEAK_MAX; i++) {
		if (mem_info[i].ptr != NULL) {
#ifdef DEBUG
			printf("Detected memory leak!\n");
			printf(" address: %p\n", mem_info[i].ptr);
			printf(" size: %u\n", (unsigned int)mem_info[i].size);
			printf(" code: %s:%u\n", mem_info[i].file, mem_info[i].line);
			printf("\n");
#endif
			leak_detect_free(mem_info[i].ptr);
		}
	}
}

static void leak_detect_init()
{
	for (int i=0; i<LEAK_MAX; i++) {
		mem_info[i].ptr = NULL;
		mem_info[i].size = 0;
		mem_info[i].file = NULL;
		mem_info[i].line = 0;
	}
	atexit(leak_detect_check);
}

static void *leak_detect_malloc(size_t size, const char *file, unsigned int line)
{
	if (!leak_detect_initialized) {
		leak_detect_init();
		leak_detect_initialized = 1;
	}

	void *ptr = _malloc(size);
	if (ptr == NULL) {
		printf("leak_detect_malloc failed!\n");
		return NULL;
	}

	for (int i=0; i<LEAK_MAX; i++) {
		if (mem_info[i].ptr == NULL) {
			mem_info[i].ptr = ptr;
			mem_info[i].size = size;
			mem_info[i].file = file;
			mem_info[i].line = line;
			break;
		}
	}
	return ptr;
}

static void *leak_detect_calloc(int n, size_t size, const char *file, unsigned int line)
{
	void *p = leak_detect_malloc(n*size, file, line);
	memset(p, 0, n*size);
	return p;
}

#define LEAK_DETECT
#ifdef LEAK_DETECT
#define leak_init()	leak_detect_init()
#define malloc(s)	leak_detect_malloc(s, __FILE__, __LINE__)
#define calloc(n, s)	leak_detect_calloc(n, s, __FILE__, __LINE__)
#define free		leak_detect_free
#define leak_check()	leak_detect_check()
#else
#define leak_init()
#define leak_check()
#endif
