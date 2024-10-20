#pragma once
#include <Eendgine/entityBatches.hpp>
#include <filesystem>
#include <optional>
#include <vector>
namespace Eend = Eendgine;

class Terrain {
  public:
    Terrain(std::filesystem::path pngHeightMap, glm::vec3 scale);
    ~Terrain();

  private:
    int _height;
    int _width;
    std::vector<std::vector<int>> _heightMap;
    Eend::ModelId _modelId;
};
