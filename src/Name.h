#ifndef AQUA_NAME_H
#define AQUA_NAME_H

#include <string>

namespace aqua {
class Name {
  public:
    Name() = default;

    explicit Name(std::string_view str);

    [[nodiscard]] uint32_t Id() const;
    [[nodiscard]] bool IsNone() const;
    [[nodiscard]] std::string ToString() const;

    auto operator<=>(const Name &) const = default;

  private:
    uint32_t id_ = 0;
};
} // namespace aqua

template <> struct std::hash<aqua::Name> {
    size_t operator()(const aqua::Name &n) const noexcept { return std::hash<uint32_t>{}(n.Id()); }
};

#endif // AQUA_NAME_H