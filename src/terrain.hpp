#pragma once
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <filesystem>
#include <vector>
namespace Eend = Eendgine;

class Terrain {
    public:
        Terrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        ~Terrain();

        int getHeight() { return _height; };
        int getWidth() { return _width; };

        float heightAtPoint(float x, float y);

    private:
        int _height;
        int _width;
        std::vector<std::vector<float>> _heightMap;
        Eend::StatueId _statueId;
        std::vector<Eend::BoardId> _boards;
        glm::vec3 _scale;
};
