#ifndef AQUA_OBJECT_H
#define AQUA_OBJECT_H

#include "TypeInfo.h"
#include "TypeRegistry.h"

namespace aqua {

class Object {
  public:
    using Self = Object;
    using Super = Object;

    virtual ~Object() = default;

    [[nodiscard]] virtual TypeInfo *GetClass() const;
    static TypeInfo *StaticClass();

  private:
    static TypeInfo *typeInfo;
    static TypeAutoRegistrar autoRegister_;

    static void CreateReflection();
    static void BindReflection();
};

} // namespace aqua

#endif // AQUA_OBJECT_H
