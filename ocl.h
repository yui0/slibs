//---------------------------------------------------------
//	Cat's eye
//
//		©2016-2018 Yuichiro Nakada
//---------------------------------------------------------

#include <stdio.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define _STRGF(x)	# x
#define OCLSTRINGIFY(x)	_STRGF(x)

#define MAX_PLATFORMS	10
#define MAX_DEVICES	10

#define OCL_READ	1
#define OCL_WRITE	2
#define OCL_WRITE_ONCE	4

#ifdef _DEBUG
#define checkOcl(err) __checkOclErrors((err), #err, __FILE__, __LINE__)
static void __checkOclErrors(const cl_int err, const char* const func, const char* const file, const int line)
{
	if (err != CL_SUCCESS) {
		fprintf(stderr, "OpenCL error at %s:%d code=%d \"%s\" \n", file, line, err, func);
	}
}
#else
#define checkOcl(x) x
#endif

typedef struct {
	int type;
	int size;
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
cl_context context;
cl_command_queue command_queue;

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

	ret = clGetPlatformIDs(MAX_PLATFORMS, platform_id, &num_platforms);
	ret = clGetDeviceIDs(platform_id[platform], type, MAX_DEVICES, device_id, &num_devices);

	// device name (option)
	size_t size;
	char str[256];
	clGetDeviceInfo(device_id[device], CL_DEVICE_NAME, sizeof(str), str, &size);
	printf("%s (platform %d, device %d)\n", str, platform, device);

	context = clCreateContext(NULL, 1, &device_id[device], NULL, NULL, &ret);
//#if ! defined(CL_VERSION_2_0)
	command_queue = clCreateCommandQueue(context, device_id[device], 0, &ret);
//#else
	/*cl_queue_properties queueProps[] =
		{ CL_QUEUE_PROPERTIES, (CL_QUEUE_ON_DEVICE && CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE), 0 };*/
//	command_queue = clCreateCommandQueueWithProperties(context, device_id[device], /*queueProps*/0, &ret);
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

	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&src, 0, &ret);
	ret = clBuildProgram(program, 1, &device_id[ocl_device], NULL, NULL, NULL);
	if (ret) {
		size_t len = 0;
		cl_int ret = CL_SUCCESS;
		ret = clGetProgramBuildInfo(program, device_id[ocl_device], CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
		char *buffer = calloc(len, sizeof(char));
		ret = clGetProgramBuildInfo(program, device_id[ocl_device], CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
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
				if (!args->p) args->p = clCreateBuffer(context, args->type, args->size, NULL, &ret);
				if (!args->p) printf("clCreateBuffer error!! %d\n", ret);
			}
			args++;
		}
		kernel++;
	}
}

void oclKernelArgsWrite(args_t *args)
{
	while (args->size) {
		if (args->flag & OCL_WRITE) {
			clEnqueueWriteBuffer(command_queue, args->p, CL_TRUE, 0, args->size, args->s, 0, 0, 0);
			if (args->flag & OCL_WRITE_ONCE) args->flag ^= OCL_WRITE;
//			printf("clEnqueueWriteBuffer size:%d %x\n", args->size, args->s);
		}
		args++;
	}
}

void oclKernelArgsRead(args_t *args)
{
	while (args->size) {
		if (args->flag & OCL_READ) {
			clEnqueueReadBuffer(command_queue, args->p, CL_TRUE, 0, args->size, args->s, 0, 0, 0);
//			printf("clEnqueueReadBuffer size:%d %x\n", args->size, args->s);
		}
		args++;
	}
}

void oclRun(ocl_t *kernel)
{
	int n = 0;
	args_t *args = kernel->a;
	while (args->size) {
#ifdef _DEBUG
		printf("clSetKernelArg:[%d] %d (%x/%d) %x\n", n, sizeof(cl_mem), (void*)args->p, args->size, (void*)args->s);
#endif
		if (args->type>0) checkOcl(clSetKernelArg(kernel->k, n++, sizeof(cl_mem), (void*)&args->p));
		else checkOcl(clSetKernelArg(kernel->k, n++, args->size, (void*)args->s));
		args++;
	}

	size_t *local = kernel->local_size[0] ? kernel->local_size : 0;
	checkOcl(clEnqueueNDRangeKernel(command_queue, kernel->k, kernel->dim, NULL, kernel->global_size, local, 0, NULL, NULL));
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

void oclFinish()
{
	clFlush(command_queue);
	clFinish(command_queue);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
}
