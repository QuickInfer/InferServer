#ifndef CUDA_H
#define CUDA_H

#include "Device.h"

// CUDA 子类
class CUDA : public Device {
public:
    CUDA();
    void move_in(float* ptr_dev, float* ptr_cpu, size_t bytes) override;
    void move_out(float* ptr_dev, float* ptr_cpu, size_t bytes) override;
    void allocate(float* ptr, size_t size) override;
    void deallocate(float* ptr) override;
};

#endif // CUDA_H