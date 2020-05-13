//---------------------------------------------------------
//	Cat's eye
//
//		©2016-2020 Yuichiro Nakada
//---------------------------------------------------------

#ifndef OCL_H_INCLUDED
#define OCL_H_INCLUDED

#include <stdio.h>
#include <string.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define _OCL_(...)	# __VA_ARGS__
#define _STRGF(x)	# x
#define OCLSTRINGIFY(x)	_STRGF(x)

#define MAX_PLATFORMS	10
#define MAX_DEVICES	10

#define OCL_OUTPUT	1
#define OCL_INPUT	2
#define OCL_INPUT_ONCE	4
#define OCL_BUFFER	8
// deprecated
#define OCL_READ	1
#define OCL_WRITE	2
#define OCL_WRITE_ONCE	4

//#define _DEBUG
#ifdef _DEBUG
#define checkOcl(err) __checkOclErrors((err), #err, __FILE__, __LINE__)
static void __checkOclErrors(const cl_int err, const char* const func, const char* const file, const int line)
{
	if (err != CL_SUCCESS) {
		fprintf(stderr, "OpenCL error at %s:%d code=%d \"%s\" \n", file, line, err, func);

		switch (err) {
		case CL_DEVICE_NOT_FOUND:                 printf("-- Error at %d:  Device not found.\n", line); break;
		case CL_DEVICE_NOT_AVAILABLE:             printf("-- Error at %d:  Device not available\n", line); break;
		case CL_COMPILER_NOT_AVAILABLE:           printf("-- Error at %d:  Compiler not available\n", line); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:    printf("-- Error at %d:  Memory object allocation failure\n", line); break;
		case CL_OUT_OF_RESOURCES:                 printf("-- Error at %d:  Out of resources\n", line); break;
		case CL_OUT_OF_HOST_MEMORY:               printf("-- Error at %d:  Out of host memory\n", line); break;
		case CL_PROFILING_INFO_NOT_AVAILABLE:     printf("-- Error at %d:  Profiling information not available\n", line); break;
		case CL_MEM_COPY_OVERLAP:                 printf("-- Error at %d:  Memory copy overlap\n", line); break;
		case CL_IMAGE_FORMAT_MISMATCH:            printf("-- Error at %d:  Image format mismatch\n", line); break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:       printf("-- Error at %d:  Image format not supported\n", line); break;
		case CL_BUILD_PROGRAM_FAILURE:            printf("-- Error at %d:  Program build failure\n", line); break;
		case CL_MAP_FAILURE:                      printf("-- Error at %d:  Map failure\n", line); break;
		case CL_INVALID_VALUE:                    printf("-- Error at %d:  Invalid value\n", line); break;
		case CL_INVALID_DEVICE_TYPE:              printf("-- Error at %d:  Invalid device type\n", line); break;
		case CL_INVALID_PLATFORM:                 printf("-- Error at %d:  Invalid platform\n", line); break;
		case CL_INVALID_DEVICE:                   printf("-- Error at %d:  Invalid device\n", line); break;
		case CL_INVALID_CONTEXT:                  printf("-- Error at %d:  Invalid context\n", line); break;
		case CL_INVALID_QUEUE_PROPERTIES:         printf("-- Error at %d:  Invalid queue properties\n", line); break;
		case CL_INVALID_COMMAND_QUEUE:            printf("-- Error at %d:  Invalid command queue\n", line); break;
		case CL_INVALID_HOST_PTR:                 printf("-- Error at %d:  Invalid host pointer\n", line); break;
		case CL_INVALID_MEM_OBJECT:               printf("-- Error at %d:  Invalid memory object\n", line); break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  printf("-- Error at %d:  Invalid image format descriptor\n", line); break;
		case CL_INVALID_IMAGE_SIZE:               printf("-- Error at %d:  Invalid image size\n", line); break;
		case CL_INVALID_SAMPLER:                  printf("-- Error at %d:  Invalid sampler\n", line); break;
		case CL_INVALID_BINARY:                   printf("-- Error at %d:  Invalid binary\n", line); break;
		case CL_INVALID_BUILD_OPTIONS:            printf("-- Error at %d:  Invalid build options\n", line); break;
		case CL_INVALID_PROGRAM:                  printf("-- Error at %d:  Invalid program\n", line); break;
		case CL_INVALID_PROGRAM_EXECUTABLE:       printf("-- Error at %d:  Invalid program executable\n", line); break;
		case CL_INVALID_KERNEL_NAME:              printf("-- Error at %d:  Invalid kernel name\n", line); break;
		case CL_INVALID_KERNEL_DEFINITION:        printf("-- Error at %d:  Invalid kernel definition\n", line); break;
		case CL_INVALID_KERNEL:                   printf("-- Error at %d:  Invalid kernel\n", line); break;
		case CL_INVALID_ARG_INDEX:                printf("-- Error at %d:  Invalid argument index\n", line); break;
		case CL_INVALID_ARG_VALUE:                printf("-- Error at %d:  Invalid argument value\n", line); break;
		case CL_INVALID_ARG_SIZE:                 printf("-- Error at %d:  Invalid argument size\n", line); break;
		case CL_INVALID_KERNEL_ARGS:              printf("-- Error at %d:  Invalid kernel arguments\n", line); break;
		case CL_INVALID_WORK_DIMENSION:           printf("-- Error at %d:  Invalid work dimensionsension\n", line); break;
		case CL_INVALID_WORK_GROUP_SIZE:          printf("-- Error at %d:  Invalid work group size\n", line); break;
		case CL_INVALID_WORK_ITEM_SIZE:           printf("-- Error at %d:  Invalid work item size\n", line); break;
		case CL_INVALID_GLOBAL_OFFSET:            printf("-- Error at %d:  Invalid global offset\n", line); break;
		case CL_INVALID_EVENT_WAIT_LIST:          printf("-- Error at %d:  Invalid event wait list\n", line); break;
		case CL_INVALID_EVENT:                    printf("-- Error at %d:  Invalid event\n", line); break;
		case CL_INVALID_OPERATION:                printf("-- Error at %d:  Invalid operation\n", line); break;
		case CL_INVALID_GL_OBJECT:                printf("-- Error at %d:  Invalid OpenGL object\n", line); break;
		case CL_INVALID_BUFFER_SIZE:              printf("-- Error at %d:  Invalid buffer size\n", line); break;
		case CL_INVALID_MIP_LEVEL:                printf("-- Error at %d:  Invalid mip-map level\n", line); break;
		default:                                  printf("-- Error at %d:  Unknown with code %d\n", line, err);
		}
	}
}
#else
#define checkOcl(x) x
#endif

typedef struct {
	int type;
	size_t size;
	cl_mem p;	// device memory
	void *s;	// cpu memory
	int flag;
} args_t;

typedef struct {
	char *f;
	cl_kernel k;
	int dim;
	size_t global_size[3];
	size_t local_size[3];
	args_t *a;
} ocl_t;

int ocl_device;
cl_device_id device_id[MAX_DEVICES];
cl_context ocl_context;
cl_command_queue command_queue;
cl_event ocl_e;

#ifdef _WIN32
char *_getenv(char *environment_name)
{
	size_t buf;
	static char buffer[1024];
	if (getenv_s(&buf, buffer, 1024, environment_name)) return 0;
	if (buf == 0) return 0;
	return buffer;
}
#else
#define _getenv	getenv
#endif

// for OpenCL 1.x
inline int ceil_int_div(int i, int div)
{
	return (i + div - 1) / div;
}

inline int ceil_int(int i, int div)
{
	return ceil_int_div(i, div) * div;
}

void oclSetup(int platform, int device)
{
	cl_platform_id platform_id[MAX_PLATFORMS];
	cl_uint num_devices;
	cl_uint num_platforms;
	cl_int ret;

	ocl_device = device;

	int type = CL_DEVICE_TYPE_ALL;
	if (_getenv("FORCE_GPU")) {
		type = CL_DEVICE_TYPE_GPU;
	} else if (_getenv("FORCE_CPU")) {
		type = CL_DEVICE_TYPE_CPU;
	} else if (_getenv("FORCE_ACCELERATOR")) {
		type = CL_DEVICE_TYPE_ACCELERATOR;
	}

	checkOcl(ret = clGetPlatformIDs(MAX_PLATFORMS, platform_id, &num_platforms));
	checkOcl(ret = clGetDeviceIDs(platform_id[platform], type, MAX_DEVICES, device_id, &num_devices));

	// device name (option)
	size_t size;
	char str[256];
	clGetDeviceInfo(device_id[device], CL_DEVICE_NAME, sizeof(str), str, &size);
	printf("%s (platform %d/%d, device %d/%d)\n", str, platform, num_platforms, device, num_devices);

	ocl_context = clCreateContext(NULL, 1, &device_id[device], NULL, NULL, &ret);
//#if ! defined(CL_VERSION_2_0)
	command_queue = clCreateCommandQueue(ocl_context, device_id[device], 0, &ret);
//#else
	/*cl_queue_properties queueProps[] =
		{ CL_QUEUE_PROPERTIES, (CL_QUEUE_ON_DEVICE && CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE), 0 };*/
//	command_queue = clCreateCommandQueueWithProperties(ocl_context, device_id[device], /*queueProps*/0, &ret);
//#endif

#ifdef _DEBUG
	size_t max_work_group_size;
	clGetDeviceInfo(device_id[device], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, NULL);
	printf("CL_DEVICE_MAX_WORK_GROUP_SIZE: %lu\n", max_work_group_size);
	size_t max_work_item_sizes[3];
	clGetDeviceInfo(device_id[device], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*3, max_work_item_sizes, NULL);
	printf("CL_DEVICE_MAX_WORK_ITEM_SIZES: "); for (size_t i=0; i<3; ++i) printf("%lu ", max_work_item_sizes[i]); printf("\n");
/*	size_t _size;
	clGetDeviceInfo(device_id[device], CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(_size), &_size, NULL);
	printf("CL_DEVICE_MAX_CONSTANT_ARGS: %lu\n", _size);*/
#endif

	cl_ulong maxMemAlloc;
	clGetDeviceInfo(device_id[device], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &maxMemAlloc, NULL);
	printf("Maximum memory allocation size is %lu bytes\n", maxMemAlloc);
}

void oclKernel(ocl_t *kernel, int n, char *opt, char *kernel_code)
{
	cl_int ret;
	const char* src[1] = { kernel_code };

	cl_program program = clCreateProgramWithSource(ocl_context, 1, (const char **)&src, 0, &ret);
	ret = clBuildProgram(program, 1, &device_id[ocl_device], NULL, NULL, NULL);
	if (ret) {
		size_t len = 0;
		cl_int ret = CL_SUCCESS;
		ret = clGetProgramBuildInfo(program, device_id[ocl_device], CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
		char *buffer = calloc(len, sizeof(char));
		ret = clGetProgramBuildInfo(program, device_id[ocl_device], CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
		printf("\n%s\n", kernel_code);
		printf("\n%s\n", buffer);
	}
	for (int i=0; i<n; i++) {
		kernel->k = clCreateKernel(program, kernel->f, &ret);

		if (!kernel->global_size[0]) {
			size_t *local = kernel->global_size;
			clGetKernelWorkGroupInfo(kernel->k, device_id[ocl_device], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t)*3, local, NULL);
			printf("CL_KERNEL_WORK_GROUP_SIZE: %zu\n", local[0]);
		}
		kernel++;
	}
	clReleaseProgram(program);
}

void oclKernelArgs(ocl_t *kernel, int n)
{
	cl_int ret;
	for (int i=0; i<n; i++) {
		args_t *args = kernel->a;
		while (args->size) {
			if (args->type>0) {
				if (!args->p) args->p = clCreateBuffer(ocl_context, args->type, args->size, NULL, &ret);
				if (!args->p) printf("clCreateBuffer error!! %d\n", ret);
			}
			args++;
		}
		kernel++;
	}
}

static inline void oclKernelArgsWrite(args_t *args)
{
	while (args->size) {
		if (args->flag & OCL_INPUT) {
			/*if (args->type & CL_MEM_ALLOC_HOST_PTR) {
				void *p = clEnqueueMapBuffer(command_queue, args->p, CL_FALSE, CL_MAP_WRITE, 0, args->size, 0, NULL, NULL, NULL);
				memcpy(p, args->s, args->size);
				clEnqueueUnmapMemObject(command_queue, args->p, p, 0, NULL, NULL);
			} else {*/
				checkOcl(clEnqueueWriteBuffer(command_queue, args->p, CL_TRUE, 0, args->size, args->s, 0, 0, 0));
				if (args->flag & OCL_INPUT_ONCE) args->flag ^= OCL_INPUT;
//				printf("clEnqueueWriteBuffer size:%d %x\n", args->size, args->s);
			//}
		}
		args++;
	}
}

static inline void oclKernelArgsRead(args_t *args)
{
	while (args->size) {
		if (args->flag & OCL_OUTPUT) {
			/*if (args->type & CL_MEM_ALLOC_HOST_PTR) {
				void *p = clEnqueueMapBuffer(command_queue, args->p, CL_FALSE, CL_MAP_READ, 0, args->size, 0, NULL, NULL, NULL);
				memcpy(args->s, p, args->size);
				clEnqueueUnmapMemObject(command_queue, args->p, p, 0, NULL, NULL);
			} else {*/
				checkOcl(clEnqueueReadBuffer(command_queue, args->p, CL_TRUE, 0, args->size, args->s, 0, 0, 0));
//				printf("clEnqueueReadBuffer size:%d %x\n", args->size, args->s);
			//}
		}
		args++;
	}
}

static inline void oclWrite(cl_mem mem, size_t offset, size_t size, void *p)
{
	checkOcl(clEnqueueWriteBuffer(command_queue, mem, CL_TRUE, offset, size, p, 0, 0, 0));
}
static inline void oclRead(cl_mem mem, size_t offset, size_t size, void *p)
{
	checkOcl(clEnqueueReadBuffer(command_queue, mem, CL_TRUE, offset, size, p, 0, 0, 0));
}

static inline void oclRun(ocl_t *kernel)
{
	int n = 0;
	args_t *args = kernel->a;
	while (args->size) {
#ifdef _DEBUG
		printf("clSetKernelArg[%d]: size %lu %x\n", n, /*sizeof(cl_mem), (unsigned int)args->p,*/ args->size, (unsigned int)args->s);
#endif
		if (args->type>0) checkOcl(clSetKernelArg(kernel->k, n++, sizeof(cl_mem), (void*)&args->p));
		else checkOcl(clSetKernelArg(kernel->k, n++, args->size, (void*)args->s));
		args++;
	}

	size_t *local = kernel->local_size[0] ? kernel->local_size : 0;
	checkOcl(clEnqueueNDRangeKernel(command_queue, kernel->k, kernel->dim, NULL, kernel->global_size, local, 0, NULL, &ocl_e));
#ifdef _DEBUG
	printf("clEnqueueNDRangeKernel (%zu/%zu,%zu/%zu,%zu/%zu)\n", kernel->local_size[0], kernel->global_size[0], kernel->local_size[1], kernel->global_size[1], kernel->local_size[2], kernel->global_size[2]);
#endif
}

void oclReleaseKernel(ocl_t *kernel, int n)
{
	for (int i=0; i<n; i++) {
		args_t *args = kernel->a;
		while (args->size) {
			if (args->type>0 && args->p) {
				clReleaseMemObject(args->p);
				args->p = 0;
			}
			args++;
		}
		clReleaseKernel(kernel->k);
		kernel++;
	}
}

void oclWait()
{
	clFinish(command_queue);
}

cl_ulong oclTime()
{
	clWaitForEvents(1, &ocl_e);
	cl_ulong start, end;
	clGetEventProfilingInfo(ocl_e, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
	clGetEventProfilingInfo(ocl_e, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
	clReleaseEvent(ocl_e);
	return end-start;
}

void oclFinish()
{
	clFlush(command_queue);
	clFinish(command_queue);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(ocl_context);
}

#endif /* OCL_H_INCLUDED */
