#include "TypeInfo.h"

namespace aqua {

TypeInfo::TypeInfo(Name name, size_t size, Factory factory)
    : name_(name), size_(size), factory_(std::move(factory)) {}

Name TypeInfo::GetName() const { return name_; }
size_t TypeInfo::GetSize() const { return size_; }
TypeInfo *TypeInfo::GetParent() const { return parent_; }

void TypeInfo::SetParent(TypeInfo *parent) { parent_ = parent; }

void TypeInfo::AddProperty(std::unique_ptr<PropertyInfo> prop) {
    properties_.push_back(std::move(prop));
}

void TypeInfo::AddFunction(std::unique_ptr<FunctionInfo> func) {
    functions_.push_back(std::move(func));
}

PropertyInfo *TypeInfo::FindOwnProperty(Name name) {
    for (auto &p : properties_) {
        if (p->GetName() == name) {
            return p.get();
        }
    }
    return nullptr;
}

FunctionInfo *TypeInfo::FindOwnFunction(Name name) {
    for (auto &f : functions_) {
        if (f->GetName() == name) {
            return f.get();
        }
    }
    return nullptr;
}

PropertyInfo *TypeInfo::FindProperty(Name name) {
    for (auto *ti = this; ti != nullptr; ti = ti->parent_) {
        if (auto *p = ti->FindOwnProperty(name)) {
            return p;
        }
    }
    return nullptr;
}

FunctionInfo *TypeInfo::FindFunction(Name name) {
    for (auto *ti = this; ti != nullptr; ti = ti->parent_) {
        if (auto *f = ti->FindOwnFunction(name)) {
            return f;
        }
    }
    return nullptr;
}

std::vector<const PropertyInfo *> TypeInfo::GetAllProperties() const {
    std::vector<const PropertyInfo *> result;
    CollectProperties(result);
    return result;
}

std::vector<const FunctionInfo *> TypeInfo::GetAllFunctions() const {
    std::vector<const FunctionInfo *> result;
    CollectFunctions(result);
    return result;
}

bool TypeInfo::IsA(const TypeInfo *target) const {
    for (const auto *ti = this; ti != nullptr; ti = ti->parent_) {
        if (ti == target) {
            return true;
        }
    }
    return false;
}

bool TypeInfo::IsA(Name targetName) const {
    for (const auto *ti = this; ti != nullptr; ti = ti->parent_) {
        if (ti->name_ == targetName) {
            return true;
        }
    }
    return false;
}

void *TypeInfo::CreateInstance() const {
    if (!factory_) {
        return nullptr;
    }
    return factory_();
}

void TypeInfo::CollectProperties(std::vector<const PropertyInfo *> &result) const {
    if (parent_) {
        parent_->CollectProperties(result);
    }
    for (const auto &p : properties_) {
        result.push_back(p.get());
    }
}

void TypeInfo::CollectFunctions(std::vector<const FunctionInfo *> &result) const {
    if (parent_) {
        parent_->CollectFunctions(result);
    }
    for (const auto &f : functions_) {
        result.push_back(f.get());
    }
}

} // namespace aqua
