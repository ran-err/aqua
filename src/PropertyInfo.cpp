#include "PropertyInfo.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <new>

namespace aqua {

// ==========
// PropertyInfo
// ==========

PropertyInfo::PropertyInfo(Name name, size_t offset, std::vector<Name> tags)
    : name_(name), offset_(offset), tags_(std::move(tags)) {}

PropertyInfo::~PropertyInfo() = default;

Name PropertyInfo::GetName() const { return name_; }
size_t PropertyInfo::GetOffset() const { return offset_; }

bool PropertyInfo::HasTag(Name tag) const { return std::ranges::find(tags_, tag) != tags_.end(); }

std::vector<Name> PropertyInfo::GetTags() const { return tags_; }

void *PropertyInfo::GetValuePtr(void *base) const {
    return static_cast<std::byte *>(base) + offset_;
}

const void *PropertyInfo::GetValuePtr(const void *base) const {
    return static_cast<const std::byte *>(base) + offset_;
}

// ==========
// IntPropertyInfo
// ==========

const char *IntPropertyInfo::GetTypeName() const { return "int"; }
size_t IntPropertyInfo::GetSize() const { return sizeof(int); }
size_t IntPropertyInfo::GetAlignment() const { return alignof(int); }

void IntPropertyInfo::Construct(void *ptr) const { new (ptr) int(0); }
void IntPropertyInfo::Destroy(void * /*ptr*/) const {}

void IntPropertyInfo::CopyValue(void *dst, const void *src) const {
    *static_cast<int *>(dst) = *static_cast<const int *>(src);
}

std::string IntPropertyInfo::ToString(const void *ptr) const {
    return std::to_string(*static_cast<const int *>(ptr));
}

bool IntPropertyInfo::SetFromString(void *ptr, const std::string &str) const {
    char *end = nullptr;
    errno = 0;
    const long val = std::strtol(str.c_str(), &end, 10);
    if (end == str.c_str() || *end != '\0' || errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        return false;
    }
    *static_cast<int *>(ptr) = static_cast<int>(val);
    return true;
}

int IntPropertyInfo::GetInt(const void *obj) const {
    return *static_cast<const int *>(GetValuePtr(obj));
}

void IntPropertyInfo::SetInt(void *obj, int value) const {
    *static_cast<int *>(GetValuePtr(obj)) = value;
}

// ==========
// FloatPropertyInfo
// ==========

const char *FloatPropertyInfo::GetTypeName() const { return "float"; }
size_t FloatPropertyInfo::GetSize() const { return sizeof(float); }
size_t FloatPropertyInfo::GetAlignment() const { return alignof(float); }

void FloatPropertyInfo::Construct(void *ptr) const { new (ptr) float(0.0f); }
void FloatPropertyInfo::Destroy(void * /*ptr*/) const {}

void FloatPropertyInfo::CopyValue(void *dst, const void *src) const {
    *static_cast<float *>(dst) = *static_cast<const float *>(src);
}

std::string FloatPropertyInfo::ToString(const void *ptr) const {
    return std::to_string(*static_cast<const float *>(ptr));
}

bool FloatPropertyInfo::SetFromString(void *ptr, const std::string &str) const {
    char *end = nullptr;
    errno = 0;
    const float val = std::strtof(str.c_str(), &end);
    if (end == str.c_str() || *end != '\0' || errno == ERANGE || !isfinite(val)) {
        return false;
    }
    *static_cast<float *>(ptr) = val;
    return true;
}

float FloatPropertyInfo::GetFloat(const void *obj) const {
    return *static_cast<const float *>(GetValuePtr(obj));
}

void FloatPropertyInfo::SetFloat(void *obj, float value) const {
    *static_cast<float *>(GetValuePtr(obj)) = value;
}

// ==========
// BoolPropertyInfo
// ==========

const char *BoolPropertyInfo::GetTypeName() const { return "bool"; }
size_t BoolPropertyInfo::GetSize() const { return sizeof(bool); }
size_t BoolPropertyInfo::GetAlignment() const { return alignof(bool); }

void BoolPropertyInfo::Construct(void *ptr) const { new (ptr) bool(false); }
void BoolPropertyInfo::Destroy(void * /*ptr*/) const {}

void BoolPropertyInfo::CopyValue(void *dst, const void *src) const {
    *static_cast<bool *>(dst) = *static_cast<const bool *>(src);
}

std::string BoolPropertyInfo::ToString(const void *ptr) const {
    return *static_cast<const bool *>(ptr) ? "true" : "false";
}

bool BoolPropertyInfo::SetFromString(void *ptr, const std::string &str) const {
    *static_cast<bool *>(ptr) = (str == "true" || str == "1");
    return true;
}

bool BoolPropertyInfo::GetBool(const void *obj) const {
    return *static_cast<const bool *>(GetValuePtr(obj));
}

void BoolPropertyInfo::SetBool(void *obj, bool value) const {
    *static_cast<bool *>(GetValuePtr(obj)) = value;
}

// ==========
// 工厂函数
// ==========

std::unique_ptr<PropertyInfo> MakeIntProperty(Name name, size_t offset, std::vector<Name> tags) {
    return std::make_unique<IntPropertyInfo>(name, offset, std::move(tags));
}

std::unique_ptr<PropertyInfo> MakeFloatProperty(Name name, size_t offset, std::vector<Name> tags) {
    return std::make_unique<FloatPropertyInfo>(name, offset, std::move(tags));
}

std::unique_ptr<PropertyInfo> MakeBoolProperty(Name name, size_t offset, std::vector<Name> tags) {
    return std::make_unique<BoolPropertyInfo>(name, offset, std::move(tags));
}

} // namespace aqua
