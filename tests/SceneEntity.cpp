#include "SceneEntity.h"

#include "FunctionInfo.h"
#include "PropertyInfo.h"
#include "TypeRegistry.h"

#include <cstddef>
#include <memory>

namespace aqua::test {

TypeInfo *SceneEntity::typeInfo = nullptr;

TypeAutoRegistrar SceneEntity::autoRegister_({SceneEntity::CreateReflection,
                                              SceneEntity::BindReflection});

TypeInfo *SceneEntity::GetClass() const { return StaticClass(); }

TypeInfo *SceneEntity::StaticClass() { return typeInfo; }

void SceneEntity::CreateReflection() {
    auto owned = std::make_unique<TypeInfo>(Name("SceneEntity"), sizeof(SceneEntity),
                                            []() -> void * { return new SceneEntity(); });
    typeInfo = owned.get();
    TypeRegistry::Instance().RegisterType(std::move(owned));
}

void SceneEntity::BindReflection() {
    typeInfo->SetParent(Super::StaticClass());
    typeInfo->AddProperty(MakeIntProperty(Name("id"), offsetof(SceneEntity, id)));
    typeInfo->AddProperty(MakeBoolProperty(Name("active"), offsetof(SceneEntity, active)));

    typeInfo->AddFunction(
        std::make_unique<FunctionInfo>(Name("Activate"), [](void *object, void * /*params*/) {
            static_cast<SceneEntity *>(object)->active = true;
        }));
}

} // namespace aqua::test
