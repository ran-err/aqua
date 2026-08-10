#include "Name.h"

#include "NameRegistry.h"

namespace aqua {
Name::Name(std::string_view str) { id_ = NameRegistry::Instance().GetOrAdd(str); }

uint32_t Name::Id() const { return id_; }

bool Name::IsNone() const { return id_ == 0; }

std::string Name::ToString() const { return NameRegistry::Instance().GetString(id_); }
} // namespace aqua