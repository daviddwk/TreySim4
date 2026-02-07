#pragma once

#include "puppyMill.hpp"
#include "terrain.hpp"

#include <Eendgine/types.hpp>

#include <filesystem>
#include <memory>
#include <optional>

class Park {
        class NextTerrainParams {
            public:
                NextTerrainParams(std::filesystem::path pngHeightMap, Eend::Scale scale)
                    : pngHeightMap(pngHeightMap), scale(scale) {}

                std::filesystem::path pngHeightMap;
                Eend::Scale scale;
        };

    public:
        static void construct(std::filesystem::path pngHeightMap, Eend::Scale scale);
        static void destruct();
        static Park& get();

        // maybe have an update function so that this actually gets set at end of frame
        void update(float dt);
        void reset();

        bool colliding(Eend::Point2D point);

        float getHeight();
        float getWidth();
        Eend::Point getSpawn();

        float heightAtPoint(Eend::Point2D point);
        Eend::Point positionAtTile(Terrain::Tile tile);
        Eend::Point positionAtTile(Terrain::Tile tile, float heightOffset);

        unsigned int numDogsKilled();

        void setTerrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        // just give pointer to terrain

    private:
        Park(std::filesystem::path pngHeightMap, Eend::Scale scale);
        ~Park() = default;

        Park(const Park& other) = delete;
        Park& operator=(const Park& other) = delete;

        Park(Park&& other) = delete;
        Park& operator=(Park&& other) = delete;

        inline static Park* m_instance = nullptr;

        std::optional<NextTerrainParams> m_nextTerrain = std::nullopt;

        std::unique_ptr<Terrain> m_terrain;
        // part of park and dog spawns should be a part of the park format TODO
        std::unique_ptr<PuppyMill> m_puppyMill;
};
