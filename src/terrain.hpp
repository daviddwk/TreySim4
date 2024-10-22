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

        int getHeight() { return _height; };
        int getWidth() { return _width; };

        float heightAtPoint(float x, float y);

    private:
        int _height;
        int _width;
        std::vector<std::vector<float>> _heightMap;
        Eend::ModelId _modelId;
        glm::vec3 _scale;
};
