#ifndef AQUA_TYPE_INFO_H
#define AQUA_TYPE_INFO_H

#include "FunctionInfo.h"
#include "PropertyInfo.h"

#include <functional>
#include <memory>
#include <vector>

namespace aqua {

class TypeInfo {
  public:
    using Factory = std::function<void *()>;

    TypeInfo(Name name, size_t size, Factory factory = nullptr);

    [[nodiscard]] Name GetName() const;
    [[nodiscard]] size_t GetSize() const;
    [[nodiscard]] TypeInfo *GetParent() const;

    void SetParent(TypeInfo *parent);
    void AddProperty(std::unique_ptr<PropertyInfo> prop);
    void AddFunction(std::unique_ptr<FunctionInfo> func);

    // 查找（沿继承链向上）
    [[nodiscard]] PropertyInfo *FindProperty(Name name);
    [[nodiscard]] FunctionInfo *FindFunction(Name name);

    // 仅查找自身
    [[nodiscard]] PropertyInfo *FindOwnProperty(Name name);
    [[nodiscard]] FunctionInfo *FindOwnFunction(Name name);

    // 收集整个继承链（父类优先）
    [[nodiscard]] std::vector<const PropertyInfo *> GetAllProperties() const;
    [[nodiscard]] std::vector<const FunctionInfo *> GetAllFunctions() const;

    // 继承查询
    [[nodiscard]] bool IsA(const TypeInfo *target) const;
    [[nodiscard]] bool IsA(Name targetName) const;

    [[nodiscard]] void *CreateInstance() const;

  private:
    void CollectProperties(std::vector<const PropertyInfo *> &result) const;
    void CollectFunctions(std::vector<const FunctionInfo *> &result) const;

    Name name_;
    size_t size_;
    Factory factory_;
    TypeInfo *parent_ = nullptr;
    std::vector<std::unique_ptr<PropertyInfo>> properties_;
    std::vector<std::unique_ptr<FunctionInfo>> functions_;
};

} // namespace aqua

#endif // AQUA_TYPE_INFO_H
