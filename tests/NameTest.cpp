#include <gtest/gtest.h>

#include "Name.h"

#include <map>
#include <set>

// ============================================================
// 基本契约
// ============================================================

TEST(NameTest, SameStringSameId) {
    aqua::Name a("Player");
    aqua::Name b("Player");
    EXPECT_EQ(a.Id(), b.Id());
    EXPECT_EQ(a, b);
}

TEST(NameTest, DifferentStringDifferentId) {
    aqua::Name a("Player");
    aqua::Name b("Enemy");
    EXPECT_NE(a.Id(), b.Id());
    EXPECT_NE(a, b);
}

TEST(NameTest, DefaultConstructIsNone) {
    aqua::Name name;
    EXPECT_TRUE(name.IsNone());
    EXPECT_EQ(name.Id(), 0);
}

TEST(NameTest, EmptyStringIsNone) {
    aqua::Name name("");
    EXPECT_TRUE(name.IsNone());
    EXPECT_EQ(name.Id(), 0);
}

TEST(NameTest, NonEmptyStringIsNotNone) {
    aqua::Name name("Foo");
    EXPECT_FALSE(name.IsNone());
    EXPECT_NE(name.Id(), 0);
}

TEST(NameTest, ToStringRoundTrip) {
    aqua::Name name("Mesh.StaticMesh");
    EXPECT_EQ(name.ToString(), "Mesh.StaticMesh");
}

TEST(NameTest, DefaultConstructToStringIsEmpty) {
    aqua::Name name;
    EXPECT_EQ(name.ToString(), "");
}

// ============================================================
// 边界与特殊输入
// ============================================================

TEST(NameTest, LongString) {
    std::string long_str(1024, 'x');
    aqua::Name name(long_str);
    EXPECT_EQ(name.ToString(), long_str);
}

// Name 不限制输入字符，校验策略由上层系统负责
TEST(NameTest, SpecialCharacters) {
    aqua::Name a("hello world");
    aqua::Name b("路径/资产");
    aqua::Name c("a\tb\nc");

    EXPECT_EQ(a.ToString(), "hello world");
    EXPECT_EQ(b.ToString(), "路径/资产");
    EXPECT_EQ(c.ToString(), "a\tb\nc");
}

// 锁定行为：大量注册后 id 仍然唯一，防止未来换数据结构/哈希时引入冲突
TEST(NameTest, BulkRegistrationUnique) {
    constexpr int N = 10000;
    std::set<uint32_t> ids;
    for (int i = 0; i < N; ++i) {
        aqua::Name name("name_" + std::to_string(i));
        ids.insert(name.Id());
    }
    EXPECT_EQ(ids.size(), N);
}

// 锁定行为：Name 区分大小写，确保未来不会意外引入 case-insensitive 比较
TEST(NameTest, CaseSensitive) {
    aqua::Name lower("player");
    aqua::Name upper("Player");
    EXPECT_NE(lower, upper);
    EXPECT_NE(lower.Id(), upper.Id());
}

// ============================================================
// 比较与容器兼容性
// ============================================================

TEST(NameTest, EqualitySymmetric) {
    aqua::Name a("Foo");
    aqua::Name b("Foo");
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, a);
}

TEST(NameTest, ComparisonTransitive) {
    // 注册顺序决定 id 大小，a < b < c
    aqua::Name a("A");
    aqua::Name b("B");
    aqua::Name c("C");
    if (a < b && b < c) {
        EXPECT_LT(a, c);
    }
}

TEST(NameTest, WorksAsSetKey) {
    aqua::Name a("X");
    aqua::Name b("Y");
    aqua::Name a2("X");

    std::set<aqua::Name> s;
    s.insert(a);
    s.insert(b);
    s.insert(a2);
    EXPECT_EQ(s.size(), 2);
}

TEST(NameTest, WorksAsMapKey) {
    aqua::Name key("health");
    std::map<aqua::Name, int> m;
    m[key] = 100;

    aqua::Name same_key("health");
    EXPECT_EQ(m[same_key], 100);
}

TEST(NameTest, CopySemantics) {
    aqua::Name original("Copy");
    aqua::Name copy = original;
    EXPECT_EQ(original, copy);
    EXPECT_EQ(original.Id(), copy.Id());
    EXPECT_EQ(copy.ToString(), "Copy");
}
