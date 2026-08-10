#ifndef AQUA_TYPE_REGISTRY_H
#define AQUA_TYPE_REGISTRY_H

#include "Name.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace aqua {

class TypeInfo;

struct TypeRegistration {
    using CreateTypeFunc = void (*)();
    using BindTypeFunc = void (*)();

    CreateTypeFunc create;
    BindTypeFunc bind;
};

struct TypeAutoRegistrar {
    TypeAutoRegistrar(TypeRegistration registration);
};

class TypeRegistry {
  public:
    static TypeRegistry &Instance();

    void AddRegistration(TypeRegistration registration);
    void RegisterType(std::unique_ptr<TypeInfo> info);
    void Initialize();

    [[nodiscard]] TypeInfo *Find(Name name) const;
    [[nodiscard]] void *CreateInstance(Name name) const;

  private:
    TypeRegistry() = default;

    std::vector<TypeRegistration> registrations_;
    std::unordered_map<Name, std::unique_ptr<TypeInfo>> types_;
    bool initialized_ = false;
};

} // namespace aqua

#endif // AQUA_TYPE_REGISTRY_H
