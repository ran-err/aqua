#include "FunctionInfo.h"
#include "Name.h"
#include "PropertyInfo.h"

#include <gtest/gtest.h>

#include <cstdint>

// -- FunctionInfo 对齐测试 --

TEST(FunctionTest, AddParam_OffsetsRespectAlignment) {
    // 布局: int(4,align4) -> bool(1,align1) -> float(4,align4)
    // 期望: int@0, bool@4, float@8
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});

    func.AddParam(aqua::MakeIntProperty(aqua::Name("a")));
    func.AddParam(aqua::MakeBoolProperty(aqua::Name("b")));
    func.AddParam(aqua::MakeFloatProperty(aqua::Name("c")));

    EXPECT_EQ(func.GetParam(0)->GetOffset(), 0u);
    EXPECT_EQ(func.GetParam(1)->GetOffset(), sizeof(int));
    // bool 之后 float 需要 4 字节对齐
    EXPECT_EQ(func.GetParam(2)->GetOffset() % alignof(float), 0u);
    EXPECT_EQ(func.GetParam(2)->GetOffset(), 8u);
}

TEST(FunctionTest, ParamsAlignment_IsMaxOfAll) {
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});

    func.AddParam(aqua::MakeBoolProperty(aqua::Name("a")));
    func.AddParam(aqua::MakeIntProperty(aqua::Name("b")));

    EXPECT_EQ(func.GetParamsAlignment(), alignof(int));
}

TEST(FunctionTest, SetReturn_OffsetAfterParams) {
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});

    func.AddParam(aqua::MakeIntProperty(aqua::Name("a")));
    func.AddParam(aqua::MakeBoolProperty(aqua::Name("b")));
    func.SetReturn(aqua::MakeFloatProperty(aqua::Name("__return")));

    const auto *ret = func.GetReturnProperty();
    EXPECT_NE(ret, nullptr);
    EXPECT_GE(ret->GetOffset(), sizeof(int) + sizeof(bool));
    EXPECT_EQ(ret->GetOffset() % alignof(float), 0u);
}

TEST(FunctionTest, IsReturnVoid) {
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});
    EXPECT_TRUE(func.IsReturnVoid());

    func.SetReturn(aqua::MakeIntProperty(aqua::Name("__return")));
    EXPECT_FALSE(func.IsReturnVoid());
}

// -- CallFrame 测试 --

TEST(FunctionTest, CallFrame_MemoryAlignment) {
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});
    func.AddParam(aqua::MakeIntProperty(aqua::Name("a")));

    aqua::CallFrame frame(func);
    auto addr = reinterpret_cast<std::uintptr_t>(frame.GetParamPtr(0));
    EXPECT_EQ(addr % func.GetParamsAlignment(), 0u);
}

TEST(FunctionTest, CallFrame_GetParamPtr) {
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});
    func.AddParam(aqua::MakeIntProperty(aqua::Name("a")));
    func.AddParam(aqua::MakeFloatProperty(aqua::Name("b")));

    aqua::CallFrame frame(func);

    // 写入后读取
    *static_cast<int *>(frame.GetParamPtr(0)) = 42;
    *static_cast<float *>(frame.GetParamPtr(1)) = 3.14f;

    EXPECT_EQ(*static_cast<const int *>(frame.GetParamPtr(0)), 42);
    EXPECT_FLOAT_EQ(*static_cast<const float *>(frame.GetParamPtr(1)), 3.14f);
}

TEST(FunctionTest, CallFrame_TypedParamRoundTrip) {
    aqua::FunctionInfo func(aqua::Name("test"), [](void *, void *) {});
    func.AddParam(aqua::MakeIntProperty(aqua::Name("a")));
    func.AddParam(aqua::MakeFloatProperty(aqua::Name("b")));

    aqua::CallFrame frame(func);
    frame.SetTypedParam<int>(0, 10);
    frame.SetTypedParam<float>(1, 2.5f);

    EXPECT_EQ(frame.GetTypedParam<int>(0), 10);
    EXPECT_FLOAT_EQ(frame.GetTypedParam<float>(1), 2.5f);
}

TEST(FunctionTest, CallFrame_InvokeWithReturn) {
    // 函数: int Add(int a, int b) -> a + b
    aqua::FunctionInfo func(aqua::Name("Add"));
    func.AddParam(aqua::MakeIntProperty(aqua::Name("a")));
    func.AddParam(aqua::MakeIntProperty(aqua::Name("b")));
    func.SetReturn(aqua::MakeIntProperty(aqua::Name("__return")));

    // invoker 通过 PropertyInfo 读写参数，不硬编码偏移量
    func.SetInvoker([&func](void * /*object*/, void *params) {
        int a = *static_cast<int *>(func.GetParam(0)->GetValuePtr(params));
        int b = *static_cast<int *>(func.GetParam(1)->GetValuePtr(params));
        *static_cast<int *>(func.GetReturnProperty()->GetValuePtr(params)) = a + b;
    });

    aqua::CallFrame frame(func);
    frame.SetTypedParam<int>(0, 3);
    frame.SetTypedParam<int>(1, 7);
    frame.Invoke(nullptr);

    EXPECT_EQ(frame.GetTypedReturn<int>(), 10);
}

TEST(FunctionTest, CallFrame_InvokeVoid) {
    int counter = 0;
    aqua::FunctionInfo func(aqua::Name("Increment"), [&counter](void *, void *) { ++counter; });

    aqua::CallFrame frame(func);
    frame.Invoke(nullptr);

    EXPECT_EQ(counter, 1);
}

TEST(FunctionTest, CallFrame_InvokeWithObject) {
    struct Counter {
        int value = 0;
    };

    aqua::FunctionInfo func(aqua::Name("Increment"), [](void *object, void *) {
        static_cast<Counter *>(object)->value += 1;
    });

    Counter c;
    aqua::CallFrame frame(func);
    frame.Invoke(&c);

    EXPECT_EQ(c.value, 1);
}
