#include "FunctionInfo.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <new>

namespace aqua {

// ==========
// FunctionInfo
// ==========

FunctionInfo::FunctionInfo(Name name) : name_(name) {}

FunctionInfo::FunctionInfo(Name name, Invoker invoker)
    : name_(name), invoker_(std::move(invoker)) {}

void FunctionInfo::SetInvoker(Invoker invoker) { invoker_ = std::move(invoker); }

Name FunctionInfo::GetName() const { return name_; }
size_t FunctionInfo::GetParamsSize() const { return paramsSize_; }
size_t FunctionInfo::GetParamsAlignment() const { return paramsAlignment_; }
size_t FunctionInfo::GetParamCount() const { return params_.size(); }

const PropertyInfo *FunctionInfo::GetParam(size_t index) const {
    assert(index < params_.size());
    return params_[index].get();
}

const PropertyInfo *FunctionInfo::GetReturnProperty() const { return returnProperty_.get(); }

bool FunctionInfo::IsReturnVoid() const { return returnProperty_ == nullptr; }

void FunctionInfo::AddParam(std::unique_ptr<PropertyInfo> param) {
    assert(param);
    const size_t align = param->GetAlignment();
    const size_t size = param->GetSize();
    paramsAlignment_ = std::max(paramsAlignment_, align);
    paramsSize_ = (paramsSize_ + align - 1) & ~(align - 1);
    param->offset_ = paramsSize_;
    paramsSize_ += size;
    params_.push_back(std::move(param));
}

void FunctionInfo::SetReturn(std::unique_ptr<PropertyInfo> ret) {
    assert(ret);
    const size_t align = ret->GetAlignment();
    const size_t size = ret->GetSize();
    paramsAlignment_ = std::max(paramsAlignment_, align);
    paramsSize_ = (paramsSize_ + align - 1) & ~(align - 1);
    ret->offset_ = paramsSize_;
    paramsSize_ += size;
    returnProperty_ = std::move(ret);
}

void FunctionInfo::Invoke(void *object, void *paramsMemory) const {
    assert(invoker_);
    invoker_(object, paramsMemory);
}

// ==========
// CallFrame
// ==========

CallFrame::CallFrame(const FunctionInfo &func) : func_(func), memory_(nullptr) {
    if (func_.GetParamsSize() > 0) {
        memory_ = ::operator new(func_.GetParamsSize(),
                                 static_cast<std::align_val_t>(func_.GetParamsAlignment()));
        std::memset(memory_, 0, func_.GetParamsSize());
    }
    ConstructAll();
}

CallFrame::~CallFrame() {
    DestroyAll();
    if (memory_) {
        ::operator delete(memory_, static_cast<std::align_val_t>(func_.GetParamsAlignment()));
    }
}

void *CallFrame::GetParamPtr(size_t index) { return func_.GetParam(index)->GetValuePtr(memory_); }

const void *CallFrame::GetParamPtr(size_t index) const {
    return func_.GetParam(index)->GetValuePtr(memory_);
}

void *CallFrame::GetReturnPtr() {
    assert(!func_.IsReturnVoid());
    return func_.GetReturnProperty()->GetValuePtr(memory_);
}

const void *CallFrame::GetReturnPtr() const {
    assert(!func_.IsReturnVoid());
    return func_.GetReturnProperty()->GetValuePtr(memory_);
}

void CallFrame::Invoke(void *object) { func_.Invoke(object, memory_); }

void CallFrame::ConstructAll() {
    for (size_t i = 0; i < func_.GetParamCount(); ++i) {
        const auto *param = func_.GetParam(i);
        param->Construct(param->GetValuePtr(memory_));
    }
    if (!func_.IsReturnVoid()) {
        const auto *ret = func_.GetReturnProperty();
        ret->Construct(ret->GetValuePtr(memory_));
    }
}

void CallFrame::DestroyAll() {
    if (!func_.IsReturnVoid()) {
        const auto *ret = func_.GetReturnProperty();
        ret->Destroy(ret->GetValuePtr(memory_));
    }
    for (size_t i = func_.GetParamCount(); i > 0; --i) {
        const auto *param = func_.GetParam(i - 1);
        param->Destroy(param->GetValuePtr(memory_));
    }
}

} // namespace aqua
