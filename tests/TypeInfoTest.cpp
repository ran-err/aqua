#include "Name.h"
#include "TestTypes.h"
#include "TypeRegistry.h"

#include <gtest/gtest.h>

using aqua::test::Player;
using aqua::test::SceneEntity;

// -- 两阶段初始化环境 --

class AquaEnvironment : public ::testing::Environment {
  public:
    void SetUp() override { aqua::TypeRegistry::Instance().Initialize(); }
};

static ::testing::Environment *const aquaEnv =
    ::testing::AddGlobalTestEnvironment(new AquaEnvironment());

// -- TypeInfo 属性查找 --

TEST(TypeInfoTest, FindOwnProperty) {
    auto *type = SceneEntity::StaticClass();
    EXPECT_NE(type->FindOwnProperty(aqua::Name("id")), nullptr);
    EXPECT_NE(type->FindOwnProperty(aqua::Name("active")), nullptr);
    EXPECT_EQ(type->FindOwnProperty(aqua::Name("health")), nullptr);
}

TEST(TypeInfoTest, FindPropertyInherited) {
    auto *type = Player::StaticClass();
    // Player 自身的属性
    EXPECT_NE(type->FindProperty(aqua::Name("health")), nullptr);
    // 继承自 SceneEntity 的属性
    EXPECT_NE(type->FindProperty(aqua::Name("id")), nullptr);
    EXPECT_NE(type->FindProperty(aqua::Name("active")), nullptr);
}

TEST(TypeInfoTest, FindOwnPropertyDoesNotSearchParent) {
    auto *type = Player::StaticClass();
    EXPECT_EQ(type->FindOwnProperty(aqua::Name("id")), nullptr);
}

TEST(TypeInfoTest, GetAllProperties_ParentFirst) {
    auto *type = Player::StaticClass();
    auto all = type->GetAllProperties();
    // SceneEntity: id, active; Player: health
    ASSERT_EQ(all.size(), 3u);
    EXPECT_EQ(all[0]->GetName(), aqua::Name("id"));
    EXPECT_EQ(all[1]->GetName(), aqua::Name("active"));
    EXPECT_EQ(all[2]->GetName(), aqua::Name("health"));
}

// -- TypeInfo::IsA --

TEST(TypeInfoTest, IsA_PointerVersion_Self) {
    auto *type = Player::StaticClass();
    EXPECT_TRUE(type->IsA(type));
}

TEST(TypeInfoTest, IsA_PointerVersion_Parent) {
    auto *player = Player::StaticClass();
    auto *base = SceneEntity::StaticClass();
    EXPECT_TRUE(player->IsA(base));
    EXPECT_FALSE(base->IsA(player));
}

TEST(TypeInfoTest, IsA_NameVersion) {
    auto *type = Player::StaticClass();
    EXPECT_TRUE(type->IsA(aqua::Name("Player")));
    EXPECT_TRUE(type->IsA(aqua::Name("SceneEntity")));
    EXPECT_FALSE(type->IsA(aqua::Name("Unknown")));
}

// -- TypeInfo::CreateInstance --

TEST(TypeInfoTest, CreateInstance) {
    auto *type = SceneEntity::StaticClass();
    void *raw = type->CreateInstance();
    ASSERT_NE(raw, nullptr);

    auto *entity = static_cast<SceneEntity *>(raw);
    entity->id = 42;
    EXPECT_EQ(entity->id, 42);

    delete entity;
}

TEST(TypeInfoTest, CreateInstance_NoFactory) {
    aqua::TypeInfo type(aqua::Name("NoFactory"), 0);
    EXPECT_EQ(type.CreateInstance(), nullptr);
}

// -- TypeInfo 函数反射 --

TEST(TypeInfoTest, FindFunction) {
    auto *type = SceneEntity::StaticClass();
    EXPECT_NE(type->FindFunction(aqua::Name("Activate")), nullptr);
    EXPECT_EQ(type->FindFunction(aqua::Name("Unknown")), nullptr);
}

TEST(TypeInfoTest, FindFunctionInherited) {
    auto *player = Player::StaticClass();
    EXPECT_NE(player->FindFunction(aqua::Name("Activate")), nullptr);
}

// -- TypeRegistry --

TEST(TypeInfoTest, Registry_RegisterAndFind) {
    auto *type = SceneEntity::StaticClass();

    auto *found = aqua::TypeRegistry::Instance().Find(aqua::Name("SceneEntity"));
    EXPECT_EQ(found, type);
}

TEST(TypeInfoTest, Registry_FindUnknown) {
    EXPECT_EQ(aqua::TypeRegistry::Instance().Find(aqua::Name("Unknown")), nullptr);
}

TEST(TypeInfoTest, Registry_CreateInstance) {
    void *raw = aqua::TypeRegistry::Instance().CreateInstance(aqua::Name("SceneEntity"));
    ASSERT_NE(raw, nullptr);

    delete static_cast<SceneEntity *>(raw);
}

// -- GetClass 动态类型查询 --

TEST(TypeInfoTest, GetClass_ReturnsDynamicType) {
    Player player;
    aqua::Object *base = &player;
    EXPECT_EQ(base->GetClass(), Player::StaticClass());
}

TEST(TypeInfoTest, GetClass_BaseType) {
    SceneEntity entity;
    aqua::Object *base = &entity;
    EXPECT_EQ(base->GetClass(), SceneEntity::StaticClass());
}
