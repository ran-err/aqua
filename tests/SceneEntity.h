#ifndef AQUA_SCENE_ENTITY_H
#define AQUA_SCENE_ENTITY_H

#include "Object.h"

namespace aqua::test {

struct SceneEntity : public Object {
    using Self = SceneEntity;
    using Super = Object;

    int id = 0;
    bool active = false;

    TypeInfo *GetClass() const override;
    static TypeInfo *StaticClass();

  private:
    static TypeInfo *typeInfo;
    static TypeAutoRegistrar autoRegister_;

    static void CreateReflection();
    static void BindReflection();
};

} // namespace aqua::test

#endif // AQUA_SCENE_ENTITY_H
