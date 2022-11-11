#define CL_FILE "Vadd.cl"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include "include/CL/opencl.h"
// #include <CL/opencl.h>

void addCPU(float *a, float *b, float *r, int n);

int main(int argc, char* argv[]){
    // Length of vectors
    if(argc < 2)
    {
        puts("usage: Vadd [N] \n");
        return 0;
    }
    unsigned int n = atoi(argv[1]);

    FILE *file_handle;
    char *file_buffer, *file_log;
    size_t file_size, log_size;
    struct timeval start, end, timer;

    // Host input vectors
    float *h_a, *h_b;
    // Host output vector
    float *h_c;

    // Device input buffers
    cl_mem d_a, d_b;
    // Device output buffer
    cl_mem d_c;

    cl_platform_id cpPlatform;  // OpenCL platform
    cl_device_id device_id;     // device ID
    cl_context context;         // context
    cl_command_queue queue;     // command queue
    cl_program program;         // program
    cl_kernel kernel;           // kernel

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(float);

    file_handle=fopen(CL_FILE, "r");
    
    if(file_handle==NULL){
        printf("Couldn't find the file");
        exit(1);
    }

    // read kernel file
    fseek(file_handle, 0, SEEK_END);
    file_size = ftell(file_handle);
    rewind(file_handle);
    file_buffer = (char*)malloc(file_size+1);
    file_buffer[file_size]='\0';
    fread(file_buffer, sizeof(char), file_size, file_handle);
    fclose(file_handle);

    // Allocate memory for each vector on host
    h_a = (float*)malloc(bytes);
    h_b = (float*)malloc(bytes);
    h_c = (float*)malloc(bytes);

    // Initialize vectors on host
    int i;
    srand(time(NULL)); // Seed 값을 시간으로 설정. Seed 는 난수를 만드는 기준.
    for(i=0; i<n; i++){
        h_a[i] = (float)rand()/RAND_MAX;
        h_b[i] = (float)rand()/RAND_MAX;
        h_c[i] = 0.0;

    }
    
    size_t globalSize, localSize, grid;
    cl_int err;

    // Number of work items in each local work group
    localSize = 64;

    // Number of total work items in each local work group
    grid = (n%localSize)? (n/localSize)+1 : n/localSize;
    globalSize = grid*localSize;

    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);

    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

    // Create a context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

    // Create a command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);

    // Create a command queue
    program = clCreateProgramWithSource(context, 1, (const char **) & file_buffer, &file_size, &err);

    // Build the program executable
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "vecAdd", &err);

    // Create the input and output arrays in device memory for our calculation
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_c = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);

    // Write our data set into the input array in device memory.
    err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, bytes, h_a, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0, bytes, h_b, 0, NULL, NULL);

    // Set the arguments to our compute kernel
    err = clSetKernelArg(kernel, 0,sizeof(cl_mem), &d_a);
    err |= clSetKernelArg(kernel, 1,sizeof(cl_mem), &d_b);
    err |= clSetKernelArg(kernel, 2,sizeof(cl_mem), &d_c);
    err |= clSetKernelArg(kernel, 3,sizeof(unsigned int), &n);

    gettimeofday(&start, NULL);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);

    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);

    // Read the results from the device
    clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, bytes, h_c, 0, NULL, NULL);

    gettimeofday(&end, NULL);

    timersub(&end, &start, &timer);
    printf("GPUtime: %lf\n", (timer.tv_usec/1000.0 + timer.tv_sec * 1000.0));

    // Release OpenCL resources
    clReleaseMemObject(d_a);
    clReleaseMemObject(d_b);
    clReleaseMemObject(d_c);

    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    gettimeofday(&start, NULL);
    
    addCPU(h_a, h_b, h_c, n);

    gettimeofday(&end, NULL);
    timersub(&end, &start, &timer);
    printf("CPUtime: %lf\n", (timer.tv_usec/1000.0 + timer.tv_sec * 10000.0));

    // release host memory
    free(h_a);
    free(h_b);
    free(h_c);

    return 0;

}

void addCPU(float *a, float *b, float *r, int n){
    int i =0;
    for(i=0; i<n; i++){
        r[i] = a[i] + b[i];
    }
}
