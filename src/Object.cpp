#include "Object.h"

#include "TypeRegistry.h"

namespace aqua {

TypeInfo *Object::typeInfo = nullptr;

TypeInfo *Object::GetClass() const { return StaticClass(); }

TypeInfo *Object::StaticClass() { return typeInfo; }

void Object::CreateReflection() {
    auto owned = std::make_unique<TypeInfo>(Name("Object"), sizeof(Object));
    typeInfo = owned.get();
    TypeRegistry::Instance().RegisterType(std::move(owned));
}

void Object::BindReflection() {
    // Object is the root type — no parent, no properties
}

TypeAutoRegistrar Object::autoRegister_({Object::CreateReflection, Object::BindReflection});

} // namespace aqua
