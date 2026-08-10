#ifndef AQUA_FUNCTION_INFO_H
#define AQUA_FUNCTION_INFO_H

#include "PropertyInfo.h"

#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>

namespace aqua {

// FunctionInfo - UE5 ProcessEvent 风格的函数描述
//
// 参数和返回值布局在一块连续内存（param block）中。
// 每个参数/返回值由 PropertyInfo 描述，offset 由 AddParam/SetReturn 自动计算。
// 调用时，invoker 直接读写 param block 内存，无需 Any/ArgList。
class FunctionInfo {
  public:
    using Invoker = std::function<void(void *object, void *params)>;

    explicit FunctionInfo(Name name);
    FunctionInfo(Name name, Invoker invoker);

    [[nodiscard]] Name GetName() const;
    [[nodiscard]] size_t GetParamsSize() const;
    [[nodiscard]] size_t GetParamsAlignment() const;
    [[nodiscard]] size_t GetParamCount() const;
    [[nodiscard]] const PropertyInfo *GetParam(size_t index) const;
    [[nodiscard]] const PropertyInfo *GetReturnProperty() const;
    [[nodiscard]] bool IsReturnVoid() const;

    void SetInvoker(Invoker invoker);
    void AddParam(std::unique_ptr<PropertyInfo> param);
    void SetReturn(std::unique_ptr<PropertyInfo> ret);
    void Invoke(void *object, void *paramsMemory) const;

  private:
    Name name_;
    Invoker invoker_;
    std::vector<std::unique_ptr<PropertyInfo>> params_;
    std::unique_ptr<PropertyInfo> returnProperty_;
    size_t paramsSize_ = 0;
    size_t paramsAlignment_ = 1;
};

// CallFrame - 参数内存块管理
//
// 分配 param block，管理构造/析构，提供反射 API 和类型化便利 API。
class CallFrame {
  public:
    explicit CallFrame(const FunctionInfo &func);
    ~CallFrame();

    CallFrame(const CallFrame &) = delete;
    CallFrame &operator=(const CallFrame &) = delete;

    // ===== Reflection API =====
    // 动态系统（编辑器、脚本绑定）通过 PropertyInfo 操作参数

    [[nodiscard]] void *GetParamPtr(size_t index);
    [[nodiscard]] const void *GetParamPtr(size_t index) const;
    [[nodiscard]] void *GetReturnPtr();
    [[nodiscard]] const void *GetReturnPtr() const;
    void Invoke(void *object);

    // ===== Convenience Typed API =====
    // 供代码生成、测试和调试使用。
    // 动态系统应优先使用 PropertyInfo + GetParamPtr()。

    template <typename T> void SetTypedParam(size_t index, T value) {
        assert(index < func_.GetParamCount());
        *static_cast<T *>(GetParamPtr(index)) = value;
    }

    template <typename T> [[nodiscard]] T GetTypedParam(size_t index) const {
        assert(index < func_.GetParamCount());
        return *static_cast<const T *>(GetParamPtr(index));
    }

    template <typename T> void SetTypedReturn(T value) {
        assert(!func_.IsReturnVoid());
        *static_cast<T *>(GetReturnPtr()) = value;
    }

    template <typename T> [[nodiscard]] T GetTypedReturn() const {
        assert(!func_.IsReturnVoid());
        return *static_cast<const T *>(GetReturnPtr());
    }

  private:
    void ConstructAll();
    void DestroyAll();

    const FunctionInfo &func_;
    void *memory_;
};

} // namespace aqua

#endif // AQUA_FUNCTION_INFO_H
