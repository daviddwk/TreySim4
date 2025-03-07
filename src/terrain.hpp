#pragma once
#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <filesystem>
#include <tuple>
#include <vector>
namespace Eend = Eendgine;

class Terrain {
    public:
        Terrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        ~Terrain();

        void update();
        bool colliding(Eend::Point2D point);

        int getHeight() { return _height; };
        int getWidth() { return _width; };

        float heightAtPoint(Eend::Point2D point);
        Eend::Point positionAtTile(float tileXIdx, float tileYIdx, float heightOffset);

    private:
        int _height;
        int _width;
        std::vector<std::vector<float>> _heightMap;
        Eend::StatueId _statueId;
        std::vector<Eend::CollisionRectangle> _collisionRectangles;
        std::vector<std::tuple<Eend::BoardId, float>> _boards;
        std::vector<Eend::StatueId> _statues;
        std::vector<std::tuple<Eend::DollId, float>> _dolls;
        glm::vec3 _scale;
};
