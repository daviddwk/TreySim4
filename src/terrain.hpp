#pragma once
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <filesystem>
#include <tuple>
#include <vector>

#include "collision.hpp"
namespace Eend = Eendgine;

class Terrain {
    public:
        Terrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        ~Terrain();

        void update();
        bool colliding(Eend::Point2D point);

        int getHeight() { return m_height; };
        int getWidth() { return m_width; };

        float heightAtPoint(Eend::Point2D point);
        Eend::Point positionAtTile(float tileXIdx, float tileYIdx, float heightOffset);

    private:
        int m_height;
        int m_width;
        std::vector<std::vector<float>> m_heightMap;
        Eend::StatueId m_statueId;
        std::vector<Eend::Rectangle> m_collisionRectangles;
        std::vector<std::tuple<Eend::BoardId, float>> m_boards;
        std::vector<Eend::StatueId> m_statues;
        std::vector<std::tuple<Eend::DollId, float>> m_dolls;
        glm::vec3 m_scale;
};
