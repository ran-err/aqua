#include "TypeRegistry.h"

#include "TypeInfo.h"

namespace aqua {

TypeAutoRegistrar::TypeAutoRegistrar(TypeRegistration registration) {
    TypeRegistry::Instance().AddRegistration(registration);
}

TypeRegistry &TypeRegistry::Instance() {
    static TypeRegistry instance;
    return instance;
}

void TypeRegistry::AddRegistration(TypeRegistration registration) {
    registrations_.push_back(registration);
}

void TypeRegistry::RegisterType(std::unique_ptr<TypeInfo> info) {
    types_[info->GetName()] = std::move(info);
}

void TypeRegistry::Initialize() {
    if (initialized_) {
        return;
    }

    initialized_ = true;

    for (const auto &reg : registrations_) {
        reg.create();
    }

    for (const auto &reg : registrations_) {
        reg.bind();
    }
}

TypeInfo *TypeRegistry::Find(Name name) const {
    auto it = types_.find(name);
    return (it != types_.end()) ? it->second.get() : nullptr;
}

void *TypeRegistry::CreateInstance(Name name) const {
    auto *info = Find(name);
    if (!info) {
        return nullptr;
    }
    return info->CreateInstance();
}

} // namespace aqua
