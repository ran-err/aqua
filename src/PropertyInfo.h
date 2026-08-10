#ifndef AQUA_PROPERTY_INFO_H
#define AQUA_PROPERTY_INFO_H

#include "Name.h"

#include <memory>
#include <string>
#include <vector>

namespace aqua {

// PropertyInfo - UE5 FProperty 风格的基类
//
// 每个子类封装一种具体类型的全部行为：
//   - 内存布局（size / alignment）
//   - 生命周期（construct / destroy）
//   - 值操作（copy / toString / fromString）
//   - 字段访问（通过 offset + GetValuePtr 定位到对象或 param block 中的值）
//
// 字段、函数参数、函数返回值都用 PropertyInfo 描述。
// 字段需要 offset（相对于对象基地址）；
// 函数参数/返回值的 offset 由 FunctionInfo::AddParam/SetReturn 自动计算。
class PropertyInfo {
  public:
    explicit PropertyInfo(Name name, size_t offset = 0, std::vector<Name> tags = {});
    virtual ~PropertyInfo();

    PropertyInfo(const PropertyInfo &) = delete;
    PropertyInfo &operator=(const PropertyInfo &) = delete;

    [[nodiscard]] Name GetName() const;
    [[nodiscard]] size_t GetOffset() const;
    [[nodiscard]] bool HasTag(Name tag) const;
    [[nodiscard]] std::vector<Name> GetTags() const;

    // 类型信息
    [[nodiscard]] virtual const char *GetTypeName() const = 0;
    [[nodiscard]] virtual size_t GetSize() const = 0;
    [[nodiscard]] virtual size_t GetAlignment() const = 0;

    // 生命周期
    virtual void Construct(void *ptr) const = 0;
    virtual void Destroy(void *ptr) const = 0;

    // 值操作
    virtual void CopyValue(void *dst, const void *src) const = 0;
    [[nodiscard]] virtual std::string ToString(const void *ptr) const = 0;
    [[nodiscard]] virtual bool SetFromString(void *ptr, const std::string &str) const = 0;

    // 指针算术
    [[nodiscard]] void *GetValuePtr(void *base) const;
    [[nodiscard]] const void *GetValuePtr(const void *base) const;

  private:
    Name name_;
    size_t offset_;
    std::vector<Name> tags_;

    friend class FunctionInfo;
};

// ==========
// IntPropertyInfo
// ==========
class IntPropertyInfo final : public PropertyInfo {
  public:
    using PropertyInfo::PropertyInfo;

    [[nodiscard]] const char *GetTypeName() const override;
    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] size_t GetAlignment() const override;

    void Construct(void *ptr) const override;
    void Destroy(void *ptr) const override;

    void CopyValue(void *dst, const void *src) const override;
    [[nodiscard]] std::string ToString(const void *ptr) const override;
    [[nodiscard]] bool SetFromString(void *ptr, const std::string &str) const override;

    [[nodiscard]] int GetInt(const void *obj) const;
    void SetInt(void *obj, int value) const;
};

// ==========
// FloatPropertyInfo
// ==========
class FloatPropertyInfo final : public PropertyInfo {
  public:
    using PropertyInfo::PropertyInfo;

    [[nodiscard]] const char *GetTypeName() const override;
    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] size_t GetAlignment() const override;

    void Construct(void *ptr) const override;
    void Destroy(void *ptr) const override;

    void CopyValue(void *dst, const void *src) const override;
    [[nodiscard]] std::string ToString(const void *ptr) const override;
    [[nodiscard]] bool SetFromString(void *ptr, const std::string &str) const override;

    [[nodiscard]] float GetFloat(const void *obj) const;
    void SetFloat(void *obj, float value) const;
};

// ==========
// BoolPropertyInfo
// ==========
class BoolPropertyInfo final : public PropertyInfo {
  public:
    using PropertyInfo::PropertyInfo;

    [[nodiscard]] const char *GetTypeName() const override;
    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] size_t GetAlignment() const override;

    void Construct(void *ptr) const override;
    void Destroy(void *ptr) const override;

    void CopyValue(void *dst, const void *src) const override;
    [[nodiscard]] std::string ToString(const void *ptr) const override;
    [[nodiscard]] bool SetFromString(void *ptr, const std::string &str) const override;

    [[nodiscard]] bool GetBool(const void *obj) const;
    void SetBool(void *obj, bool value) const;
};

// 工厂函数
std::unique_ptr<PropertyInfo> MakeIntProperty(Name name, size_t offset = 0,
                                              std::vector<Name> tags = {});
std::unique_ptr<PropertyInfo> MakeFloatProperty(Name name, size_t offset = 0,
                                                std::vector<Name> tags = {});
std::unique_ptr<PropertyInfo> MakeBoolProperty(Name name, size_t offset = 0,
                                               std::vector<Name> tags = {});

} // namespace aqua

#endif // AQUA_PROPERTY_INFO_H
