#include <cuda_runtime.h>
#include "matmul.h"
#include "common.h"

// TODO:日后要是有需要的话，给他变成 矩阵乘矩阵吧
//      初始化和销毁的工作应该交给外边运行
#ifdef USE_CUBLAS
#include <cublas_v2.h>

// 全局的 cuBLAS 句柄，可以在初始化时创建和销毁
static cublasHandle_t g_cublas_handle = nullptr;

// 初始化 cuBLAS 句柄
void init_cublas() {
    if (g_cublas_handle == nullptr) {
        cublasCreate(&g_cublas_handle);
    }
}

// 销毁 cuBLAS 句柄
void destroy_cublas() {
    if (g_cublas_handle != nullptr) {
        cublasDestroy(g_cublas_handle);
        g_cublas_handle = nullptr;
    }
}

extern "C" void matmul_cuda(float *xout, const float *x, const float *w, int n, int d) {
    // 初始化 cuBLAS 句柄，给外边运行
    // init_cublas();

    // 使用 cuBLAS 进行矩阵向量乘法
    // w (D x N) 矩阵，x (N x 1) 向量，xout (D x 1) 向量
    // cuBLAS 默认使用列主序存储，需要转置矩阵 w
    float alpha = 1.0f;
    float beta = 0.0f;
    cublasStatus_t status = cublasSgemv(
        g_cublas_handle,
        CUBLAS_OP_T,   // 对矩阵 w 进行转置
        n,             // 转置前的列数
        d,             // 转置前的行数
        &alpha,
        w,             // 输入矩阵 w
        n,             // 转置后的主维度（即 leading dimension）
        x,             // 输入向量 x
        1,
        &beta,
        xout,          // 输出向量 xout
        1
    );

    if (status != CUBLAS_STATUS_SUCCESS) {
        // 错误处理
        printf("CUBLAS matmul failed\n");
    }

    // 如果需要，可以在程序结束时销毁 cuBLAS 句柄
    // destroy_cublas();
}
#else
// CUDA 内核实现矩阵乘法
__global__ void matmul_kernel(float *xout, float *x, float *w, int n, int d) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= d)
        return;

    float sum = 0.0f;
    for (int j = 0; j < n; j++) {
        sum += w[i * n + j] * x[j];
    }
    xout[i] = sum;
}

extern "C" void matmul_cuda(float *xout, const float *x, const float *w, int n, int d) {

    // 为输入和输出数据分配设备内存
    float *d_x = nullptr, *d_w = nullptr, *d_xout = nullptr;
    size_t x_bytes = n * sizeof(float);
    size_t w_bytes = n * d * sizeof(float);
    size_t xout_bytes = d * sizeof(float);
    cudaMalloc((void**)&d_x, x_bytes);
    cudaMalloc((void**)&d_w, w_bytes);
    cudaMalloc((void**)&d_xout, xout_bytes);

    // 将数据从主机拷贝到设备
    cudaMemcpy(d_x, x, x_bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_w, w, w_bytes, cudaMemcpyHostToDevice);

    // 计算线程块和网格大小
    int blockSize = num_threads_small;
    int gridSize = (d + blockSize - 1) / blockSize;

    // 限制 gridSize，避免过多的线程块
    gridSize = min(gridSize, 1024); // 根据 GPU 的规格调整

    // 调用 CUDA 内核
    matmul_kernel<<<gridSize, blockSize>>>(d_xout, d_x, d_w, n, d);

    // 将结果从设备拷贝回主机
    cudaMemcpy(xout, d_xout, xout_bytes, cudaMemcpyDeviceToHost);

    // 释放设备内存
    cudaFree(d_x);
    cudaFree(d_w);
    cudaFree(d_xout);

}
#endif
