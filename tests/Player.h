#ifndef AQUA_PLAYER_H
#define AQUA_PLAYER_H

#include "SceneEntity.h"

namespace aqua::test {

struct Player : SceneEntity {
    using Self = Player;
    using Super = SceneEntity;

    float health = 0.0f;

    TypeInfo *GetClass() const override;
    static TypeInfo *StaticClass();

  private:
    static TypeInfo *typeInfo;
    static TypeAutoRegistrar autoRegister_;

    static void CreateReflection();
    static void BindReflection();
};

} // namespace aqua::test

#endif // AQUA_PLAYER_H
