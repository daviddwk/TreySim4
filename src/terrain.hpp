#pragma once
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <filesystem>
#include <print>
#include <tuple>
#include <vector>

namespace Eend = Eendgine;

class Terrain {
    public:
        // would like this to be one path with scale build into the format
        Terrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        ~Terrain();

        Terrain(const Terrain& other) = delete;
        Terrain& operator=(const Terrain& other) = delete;

        Terrain(Terrain&& other) = delete;
        Terrain& operator=(Terrain&& other) = delete;

        void update();
        bool colliding(Eend::Point2D point);

        float getHeight() { return static_cast<float>(m_height) * m_scale.x; }
        float getWidth() { return static_cast<float>(m_width) * m_scale.y; }
        Eend::Point getSpawn() { return positionAtTile(m_spawn); }

        float heightAtPoint(Eend::Point2D point);
        Eend::Point positionAtTile(Eend::Tile tile);
        Eend::Point positionAtTile(Eend::Tile tile, float heightOffset);

    private:
        int m_height;
        int m_width;
        std::vector<std::vector<float>> m_heightMap;
        Eend::StatueId m_statueId;
        std::vector<Eend::Rectangle> m_collisionRectangles;
        std::vector<std::tuple<Eend::BoardId, float>> m_boards;
        std::vector<Eend::StatueId> m_statues;
        std::vector<std::tuple<Eend::DollId, float>> m_dolls;
        Eend::Scale m_scale;
        Eend::Tile m_spawn;
};
