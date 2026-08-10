#ifndef AQUA_NAME_REGISTRY_H
#define AQUA_NAME_REGISTRY_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace aqua {
class NameRegistry {
  public:
    static NameRegistry &Instance();

    uint32_t GetOrAdd(std::string_view str);
    [[nodiscard]] std::string GetString(uint32_t id) const;

  private:
    NameRegistry();

    std::vector<std::string> strings_;
    std::unordered_map<std::string, uint32_t> lookup_;
};
} // namespace aqua

#endif // AQUA_NAME_REGISTRY_H