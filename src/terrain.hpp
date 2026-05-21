#pragma once

#include "playground.hpp"
#include "portal.hpp"

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include <json/json.h>

#include <chrono>
#include <filesystem>
#include <print>
#include <tuple>
#include <vector>

namespace Eend = Eendgine;

using Tile = glm::vec2;
using TileScale = glm::vec2;

class Terrain {

    public:
        // would like this to be one path with scale build into the format
        Terrain(std::filesystem::path pngHeightMap);
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

        void playgroundEnable(const std::string& playgroundName);
        void playgroundDisable(const std::string& playgroundName);
        bool playgroundIsEnabled(const std::string& playgroundName);
        void playgroundToggle(const std::string& playgroundName);

        float heightAtPoint(Eend::Point2D point);
        Eend::Point positionAtTile(Tile tile);
        Eend::Point positionAtTile(Tile tile, float heightOffset);

    private:
        std::tuple<Eend::BoardId, float> boardFromJson(Json::Value boardJson);
        Eend::StatueId statueFromJson(Json::Value statueJson);
        std::tuple<Eend::DollId, float> dollFromJson(Json::Value dollJson);

        const std::filesystem::path m_path;
        int m_height;
        int m_width;
        float m_animationSpeed;
        std::vector<std::vector<float>> m_heightMap;
        Eend::StatueId m_statueId;
        std::vector<Eend::Rectangle> m_collisionRectangles;
        std::vector<std::tuple<Eend::BoardId, float>> m_boards;
        std::vector<Eend::StatueId> m_statues;
        std::vector<std::tuple<Eend::DollId, float>> m_dolls;
        Eend::Scale m_scale;
        Tile m_spawn;
        std::vector<Portal> m_portals;
        std::map<std::string, Playground> m_playgrounds;
};
