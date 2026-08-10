#include "PropertyInfo.h"
#include "Name.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>

// -- int 基本操作 --

TEST(PropertyTest, IntConstruct_DefaultZero) {
    auto prop = aqua::MakeIntProperty(aqua::Name("x"));
    int value = 42;
    prop->Construct(&value);
    EXPECT_EQ(value, 0);
}

TEST(PropertyTest, IntToStringAndSetFromString) {
    auto prop = aqua::MakeIntProperty(aqua::Name("x"));
    int value = 123;
    EXPECT_EQ(prop->ToString(&value), "123");

    EXPECT_TRUE(prop->SetFromString(&value, "-7"));
    EXPECT_EQ(value, -7);
}

TEST(PropertyTest, IntSetFromString_InvalidInput) {
    auto prop = aqua::MakeIntProperty(aqua::Name("x"));
    int value = 42;
    EXPECT_FALSE(prop->SetFromString(&value, "abc"));
    EXPECT_EQ(value, 42);
    EXPECT_FALSE(prop->SetFromString(&value, ""));
    EXPECT_EQ(value, 42);
    EXPECT_FALSE(prop->SetFromString(&value, "12x"));
    EXPECT_EQ(value, 42);
}

TEST(PropertyTest, IntCopyValue) {
    auto prop = aqua::MakeIntProperty(aqua::Name("x"));
    int src = 99;
    int dst = 0;
    prop->CopyValue(&dst, &src);
    EXPECT_EQ(dst, 99);
}

TEST(PropertyTest, IntGetSetTyped) {
    struct Obj {
        int hp;
    };
    auto prop = std::make_unique<aqua::IntPropertyInfo>(aqua::Name("hp"), offsetof(Obj, hp));

    Obj obj{};
    prop->SetInt(&obj, 42);
    EXPECT_EQ(prop->GetInt(&obj), 42);
}

TEST(PropertyTest, IntTypeInfo) {
    auto prop = aqua::MakeIntProperty(aqua::Name("x"));
    EXPECT_STREQ(prop->GetTypeName(), "int");
    EXPECT_EQ(prop->GetSize(), sizeof(int));
    EXPECT_EQ(prop->GetAlignment(), alignof(int));
}

// -- float 基本操作 --

TEST(PropertyTest, FloatConstruct_DefaultZero) {
    auto prop = aqua::MakeFloatProperty(aqua::Name("x"));
    float value = 1.5f;
    prop->Construct(&value);
    EXPECT_FLOAT_EQ(value, 0.0f);
}

TEST(PropertyTest, FloatToStringAndSetFromString) {
    auto prop = aqua::MakeFloatProperty(aqua::Name("x"));
    float value = 0.0f;
    EXPECT_TRUE(prop->SetFromString(&value, "3.14"));
    EXPECT_NEAR(value, 3.14f, 0.01f);
}

TEST(PropertyTest, FloatSetFromString_InvalidInput) {
    auto prop = aqua::MakeFloatProperty(aqua::Name("x"));
    float value = 1.5f;
    EXPECT_FALSE(prop->SetFromString(&value, "abc"));
    EXPECT_FLOAT_EQ(value, 1.5f);
    EXPECT_FALSE(prop->SetFromString(&value, ""));
    EXPECT_FLOAT_EQ(value, 1.5f);
    EXPECT_FALSE(prop->SetFromString(&value, "3.14xyz"));
    EXPECT_FLOAT_EQ(value, 1.5f);
    EXPECT_FALSE(prop->SetFromString(&value, "nan"));
    EXPECT_FLOAT_EQ(value, 1.5f);
    EXPECT_FALSE(prop->SetFromString(&value, "inf"));
    EXPECT_FLOAT_EQ(value, 1.5f);
}

TEST(PropertyTest, FloatGetSetTyped) {
    struct Obj {
        float speed;
    };
    auto prop =
        std::make_unique<aqua::FloatPropertyInfo>(aqua::Name("speed"), offsetof(Obj, speed));

    Obj obj{};
    prop->SetFloat(&obj, 9.8f);
    EXPECT_FLOAT_EQ(prop->GetFloat(&obj), 9.8f);
}

// -- bool 基本操作 --

TEST(PropertyTest, BoolConstruct_DefaultFalse) {
    auto prop = aqua::MakeBoolProperty(aqua::Name("x"));
    bool value = true;
    prop->Construct(&value);
    EXPECT_FALSE(value);
}

TEST(PropertyTest, BoolToStringAndSetFromString) {
    auto prop = aqua::MakeBoolProperty(aqua::Name("x"));
    bool value = true;
    EXPECT_EQ(prop->ToString(&value), "true");

    EXPECT_TRUE(prop->SetFromString(&value, "false"));
    EXPECT_FALSE(value);

    // "true" 和 "1" 都解析为 true
    EXPECT_TRUE(prop->SetFromString(&value, "true"));
    EXPECT_TRUE(value);
    EXPECT_TRUE(prop->SetFromString(&value, "1"));
    EXPECT_TRUE(value);

    // "0" 解析为 false
    EXPECT_TRUE(prop->SetFromString(&value, "0"));
    EXPECT_FALSE(value);
}

TEST(PropertyTest, BoolGetSetTyped) {
    struct Obj {
        bool active;
    };
    auto prop =
        std::make_unique<aqua::BoolPropertyInfo>(aqua::Name("active"), offsetof(Obj, active));

    Obj obj{};
    prop->SetBool(&obj, true);
    EXPECT_TRUE(prop->GetBool(&obj));
}

// -- 通用特性 --

TEST(PropertyTest, HasTag) {
    aqua::Name visible("Visible");
    aqua::Name editable("Editable");
    aqua::Name hidden("Hidden");

    auto prop = aqua::MakeIntProperty(aqua::Name("x"), 0, {visible, editable});
    EXPECT_TRUE(prop->HasTag(visible));
    EXPECT_TRUE(prop->HasTag(editable));
    EXPECT_FALSE(prop->HasTag(hidden));
}

TEST(PropertyTest, GetValuePtrOffset) {
    struct Obj {
        int a;
        float b;
    };
    auto prop = aqua::MakeFloatProperty(aqua::Name("b"), offsetof(Obj, b));

    Obj obj{10, 3.14f};
    const auto *ptr = static_cast<const float *>(prop->GetValuePtr(&obj));
    EXPECT_FLOAT_EQ(*ptr, 3.14f);
}

TEST(PropertyTest, GetNameReturnsCorrectName) {
    aqua::Name n("health");
    auto prop = aqua::MakeIntProperty(n);
    EXPECT_EQ(prop->GetName(), n);
}

TEST(PropertyTest, GetTags) {
    aqua::Name visible("Visible");
    aqua::Name editable("Editable");

    auto prop = aqua::MakeIntProperty(aqua::Name("x"), 0, {visible, editable});
    auto tags = prop->GetTags();
    ASSERT_EQ(tags.size(), 2u);
    EXPECT_EQ(tags[0], visible);
    EXPECT_EQ(tags[1], editable);
}

TEST(PropertyTest, GetTags_Empty) {
    auto prop = aqua::MakeIntProperty(aqua::Name("x"));
    EXPECT_TRUE(prop->GetTags().empty());
}
