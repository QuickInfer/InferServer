#pragma once
#include <map>
#include <tuple>
#include <string>
#include <stdexcept>
#include "common.h"
#include "function.h"

template <typename FuncPtrT>
class FuncRegistry {
public:
    void Register(DeviceType device, DataType dtype, FuncPtrT ptr) {
        registry_map_[std::make_tuple(device, dtype)] = ptr;
    }

    FuncPtrT Get(DeviceType device, DataType dtype) const {
        auto it = registry_map_.find(std::make_tuple(device, dtype));
        if (it != registry_map_.end()) {
            return it->second;
        }
        // 更友好的错误提示
        throw std::runtime_error("Function implementation not found for device/dtype combination.");
        return nullptr;
    }

private:
    std::map<std::tuple<DeviceType, DataType>, FuncPtrT> registry_map_;
};

// 全局的注册表管理器单例
// 它为每种操作类型持有一个专门的注册表实例
class OpRegistry {
public:
    static OpRegistry& Instance() {
        static OpRegistry instance;
        return instance;
    }
    
    // 禁止拷贝和赋值
    OpRegistry(const OpRegistry&) = delete;
    OpRegistry& operator=(const OpRegistry&) = delete;

    // 提供对特定类型函数指针注册表的访问
    FuncRegistry<LinearFuncPtr>& Linear() { return linear_registry_; }
    FuncRegistry<RoPEFuncPtr>& RoPE() { return rope_registry_; }
    FuncRegistry<SiluFuncPtr>& Softmax() { return softmax_registry_; }
    FuncRegistry<SiluFuncPtr>& Silu() { return silu_registry_; }
    FuncRegistry<MultiplyElementWiseFuncPtr>& MultiplyElem() { return multiply_elem_registry_; }
    FuncRegistry<AddElementWiseFuncPtr>& AddElem() { return add_elem_registry_; }
    FuncRegistry<RMSNormFuncPtr>& RMSNorm() { return rms_norm_registry_; }
    FuncRegistry<MaxIndexFuncPtr>& MaxIndex() { return max_index_registry_; }
    // FuncRegistry<AttentionFuncPtr>& Attention() { return attention_registry_; }
    // FuncRegistry<RMSNormFuncPtr>& RMSNorm() { return rmsnorm_registry_; }
    // ...

private:
    OpRegistry() = default;

    FuncRegistry<LinearFuncPtr> linear_registry_;
    FuncRegistry<RoPEFuncPtr> rope_registry_;
    FuncRegistry<SoftmaxFuncPtr> softmax_registry_;
    FuncRegistry<SiluFuncPtr> silu_registry_;
    FuncRegistry<MultiplyElementWiseFuncPtr> multiply_elem_registry_;
    FuncRegistry<AddElementWiseFuncPtr> add_elem_registry_;
    FuncRegistry<RMSNormFuncPtr> rms_norm_registry_;
    FuncRegistry<MaxIndexFuncPtr> max_index_registry_;
    // FuncRegistry<AttentionFuncPtr> attention_registry_;
    // FuncRegistry<RMSNormFuncPtr> rmsnorm_registry_;
    // ...
};

// 通用的注册宏
#define REGISTER_OP_FUNCTION(OP_NAME, DEVICE, DTYPE, FUNC_PTR) \
    namespace { \
        struct Registrar_##OP_NAME##_##DEVICE##_##DTYPE { \
            Registrar_##OP_NAME##_##DEVICE##_##DTYPE() { \
                OpRegistry::Instance().OP_NAME().Register(DEVICE, DTYPE, FUNC_PTR); \
            } \
        }; \
        static Registrar_##OP_NAME##_##DEVICE##_##DTYPE registrar_instance; \
    }
