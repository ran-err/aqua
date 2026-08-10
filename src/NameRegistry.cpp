#include "NameRegistry.h"

namespace aqua {
NameRegistry &NameRegistry::Instance() {
    static NameRegistry instance;
    return instance;
}

NameRegistry::NameRegistry() {
    strings_.emplace_back("");
    lookup_.emplace("", 0);
}

uint32_t NameRegistry::GetOrAdd(std::string_view str) {
    if (const auto it = lookup_.find(std::string(str)); it != lookup_.end()) {
        return it->second;
    }

    auto id = static_cast<uint32_t>(strings_.size());

    strings_.emplace_back(str);
    lookup_.emplace(strings_.back(), id);

    return id;
}

std::string NameRegistry::GetString(uint32_t id) const { return strings_.at(id); }
} // namespace aqua