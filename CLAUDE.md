
# Development

开发中使用 Ninja 和 Debug 配置：

```bash
cmake -B cmake-build-debug \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug
      
cmake --build cmake-build-debug
```

# Testing

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

# Linting

```bash
clang-format --dry-run -Werror src/*.h src/*.cpp tests/*.cpp
clang-tidy src/*.cpp tests/*.cpp -p cmake-build-debug
```
