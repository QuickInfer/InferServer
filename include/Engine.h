#pragma once

#include "common.h"
#include "Worker.h"
#include <future>

class Engine
{
public:
    Engine(int num_workers, DeviceType device_type);

    ~Engine();

    void initialize_workers();

    void shutdown_workers();

    // MODIFIED to dispatch to all workers for INFER
    std::future<Result> submit_inference_request(const std::string &input_text);

private:
    int num_workers_;
    DeviceType device_type_;

    std::vector<std::unique_ptr<Worker>> workers_;
    std::atomic<uint64_t> request_id_counter_;
};