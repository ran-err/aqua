#include "Player.h"

#include "PropertyInfo.h"
#include "TypeRegistry.h"

#include <cstddef>
#include <memory>

namespace aqua::test {

TypeInfo *Player::typeInfo = nullptr;

TypeAutoRegistrar Player::autoRegister_({Player::CreateReflection, Player::BindReflection});

TypeInfo *Player::GetClass() const { return StaticClass(); }

TypeInfo *Player::StaticClass() { return typeInfo; }

void Player::CreateReflection() {
    auto owned = std::make_unique<TypeInfo>(Name("Player"), sizeof(Player),
                                            []() -> void * { return new Player(); });
    typeInfo = owned.get();
    TypeRegistry::Instance().RegisterType(std::move(owned));
}

void Player::BindReflection() {
    typeInfo->SetParent(Super::StaticClass());
    typeInfo->AddProperty(MakeFloatProperty(Name("health"), offsetof(Player, health)));
}

} // namespace aqua::test
