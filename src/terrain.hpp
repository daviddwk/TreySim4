#pragma once
#include <filesystem>
#include <optional>
#include <vector>

class Terrain {
  public:
    Terrain();
    ~Terrain();
    std::optional<std::filesystem::path> generate_terrain(std::filesystem::path png_height_map);

  private:
    int _height;
    int _width;
    std::vector<std::vector<int>> _heightMap;
};
