# Reflection System 设计文档

## 1. 背景与目标

C++ 没有原生反射能力。编辑器 Inspector、脚本绑定、序列化/反序列化等上层系统都需要在运行时查询一个类型拥有哪些属性和函数，并通过统一接口读写它们的值。反射系统为这些上层需求提供底层基础设施。

**不解决的问题：**

- 不做自动代码生成——当前采用手动注册，待 C# 工具就绪后再引入生成器
- 不处理并发安全——初始化和查询均假设单线程环境

## 2. 设计

### 2.1 当前方案概述

反射系统由四层构成，分别描述"值是什么类型"、"函数怎么调用"、"类型包含什么"、"如何注册和查询"。

#### PropertyInfo：属性描述

PropertyInfo 是属性描述的基类，每个子类封装一种具体 C++ 类型的全部行为。
基类定义统一的虚接口，新增类型只需添加一个新子类，不修改任何已有代码。

PropertyInfo 同时服务于两种场景：描述对象字段时，offset 是字段相对于对象基地址的偏移；描述函数参数时，offset 由 FunctionInfo 自动计算。

#### FunctionInfo + CallFrame：函数描述与调用

FunctionInfo 描述一个可反射调用的函数。它持有参数列表（`vector<PropertyInfo>`）、可选的返回值描述、以及一个 Invoker 回调（签名 `void(void* object, void* params)`）。

参数和返回值排列在一块连续内存（param block）中。`AddParam()` 和 `SetReturn()` 按调用顺序依次排入参数，自动处理对齐：整体对齐取所有参数的最大值，每个参数的 offset 向上对齐到该参数自身的 alignment 后记录。

CallFrame 管理 param block 的生命周期：构造时分配对齐内存并调用每个参数的 `Construct()`，析构时逆序 `Destroy()` 并释放内存。它提供两套访问 API：

- **反射 API** — `GetParamPtr(index)` / `GetReturnPtr()` 返回 `void*`，供编辑器、脚本绑定等动态系统使用
- **类型化 API** — `SetTypedParam<T>()` / `GetTypedReturn<T>()` 等模板方法，供代码生成和测试使用

#### TypeInfo：类型描述

TypeInfo 将属性列表和函数列表聚合在一个类型上，并通过 `parent_` 指针建立单继承链。

#### TypeRegistry + 两阶段初始化

TypeRegistry 是全局单例注册表。类型注册分为两个阶段，通过 `Initialize()` 一次性触发：

1. **Create phase** — 调用每个类型的 `CreateReflection()`，分配 TypeInfo 并移交所有权给 Registry
2. **Bind phase** — 调用每个类型的 `BindReflection()`，设置父类关系、添加属性和函数

两阶段分离确保 Bind 阶段所有 TypeInfo 都已存在，跨类型引用（如 `SetParent`）不会遭遇空指针。`Initialize()` 返回后，反射元数据冻结，不可再修改。

**注册机制**：每个可反射类在文件作用域声明一个 `TypeAutoRegistrar` 静态变量，其构造函数在程序启动时将 `{CreateReflection, BindReflection}` 函数对注册到 TypeRegistry。

**所有权模型**：TypeRegistry 通过 `unique_ptr` 拥有所有 TypeInfo。类中的静态 `typeInfo` 成员是不拥有所有权的观察指针。

#### Object：反射根类

Object 是所有可反射类型的根基类，提供：

- `virtual GetClass()` — 返回动态类型的 TypeInfo
- `static StaticClass()` — 返回静态类型的 TypeInfo
- `Self` / `Super` typedef — 供注册代码引用当前类和父类

Object 自身的注册实现（`CreateReflection` + `BindReflection` + `TypeAutoRegistrar`）是所有子类注册的参考模板。

#### 数据流

```
程序启动
  → TypeAutoRegistrar 构造，收集所有 {Create, Bind} 函数对
  → TypeRegistry::Initialize()
    → Create phase：分配所有 TypeInfo，赋值静态观察指针
    → Bind phase：SetParent、AddProperty、AddFunction
  → 元数据冻结
  → 运行时查询：FindProperty / FindFunction / IsA / CreateInstance
```

### 2.2 设计理由

**PropertyInfo 存储 alignment**：size 和 alignment 共同构成对象布局信息。只保存 size 不保存 alignment 意味着保存了不完整的类型信息。CallFrame 需要以正确对齐分配参数内存，缺少 alignment 会产生未定义行为。

**两阶段初始化**：分离 Create 和 Bind 解决了跨类型依赖的初始化顺序问题——Bind 阶段可以安全引用任何类型的 TypeInfo，因为 Create 阶段已经确保它们全部存在。

**Param block 无 tail padding**：struct 需要 tail padding 是为了保证数组元素对齐，而 CallFrame 只分配单块内存、不作为数组元素使用，省略 tail padding 不影响正确性。

## 3. 备选方案与取舍

**TypeTag 枚举 + switch**：用一个枚举标识属性类型，所有类型行为通过 switch 分发。实现最简单，但每添加一个新类型都需要修改枚举定义和所有 switch 分支，是典型的中心化修改问题。在类型数量增长时维护成本迅速上升。

**统一 TypeInfo 描述属性**：让 TypeInfo 同时描述属性的类型信息（类似早期 UE 的做法）。减少了类的数量，但 TypeInfo 承担过多职责，且 UE 5.25 已经弃用了这种模式，说明在实践中它并不理想。

**自动代码生成注册**：用工具扫描头文件中的标记宏，自动生成注册代码。消除手动注册的样板代码，但需要构建工具链（解析器 + 生成器），当前项目阶段引入成本过高。手动注册先把结构固定下来，代码生成作为未来演进方向。

## 4. 约束

### 4.1 前提与风险

- **单线程假设**：初始化和查询均假设单线程执行，TypeRegistry 无锁保护。多线程并发访问会导致数据竞争。
- **冻结语义**：`Initialize()` 返回后不可再添加属性、函数或父类关联。违反此约定属于未定义行为——其他类型可能已缓存继承链的查询结果，初始化顺序的保证也不再成立。
- **BoolPropertyInfo::SetFromString 宽松匹配**：仅识别 `"true"` 和 `"1"` 为 true，其余所有输入（包括拼写错误如 `"ture"`）均静默映射为 false 且返回成功。
- **PropertyInfo 子类手动实现**：每个新的属性类型都需要手动实现所有虚函数（8 个），存在遗漏或不一致的风险。

### 4.2 为什么现在可以接受

- 当前不存在并发场景，单线程假设成立。
- 冻结语义大幅简化了实现，避免了缓存失效和动态修改带来的复杂性。元数据在初始化后不可变，也为未来引入并发查询提供了天然的安全保证。
- Bool 的宽松匹配在当前无 Inspector 或反序列化消费者的情况下不会造成实际问题，待上层需求明确后再收紧语义。
- 属性类型子类数量目前只有三个（int、float、bool），手动实现的维护成本可控。

## 5. 验证

**PropertyInfo 行为契约**
- 属性包含完整布局信息
- 属性偏移可以正确定位值
- 属性支持字符串转换、无效输入拒绝

**FunctionInfo 行为契约**
- 函数签名须完整、一致
- 参数块满足每个参数的对齐要求

**TypeInfo 行为契约**
- 成员继承沿继承链进行、覆盖完整基类链
- 对象返回的动态类型与静态类型一致

## 6. 未来演进方向

- **当前的两阶段初始化应作为默认路径保留。** 冻结后的元数据具有简单、稳定的生命周期，也便于并发查询。类型卸载和重载不应破坏注册表的冻结假设。
- **继承链查询优化。** 当前采用逐级向上查找，随着查询频率或类型规模增加，可以在绑定完成后构建派生数据（如属性表的扁平缓存）来加速查询。
