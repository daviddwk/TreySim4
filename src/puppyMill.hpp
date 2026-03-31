#pragma once

#include "dog.hpp"
#include "terrain.hpp"

#include <chrono>
#include <optional>

enum class DogType { Classic };

class PuppyMill {
        class Spawn {

            public:
                Tile tile;
                std::map<
                    DogType, std::tuple<
                                 std::chrono::milliseconds,
                                 std::chrono::time_point<std::chrono::steady_clock>>>
                    timing;
                std::optional<std::chrono::milliseconds> duration;
        };

    public:
        PuppyMill(Terrain* terrain);

        void update();

        unsigned int getNumKilled();

        // TODO fix the need for this
        void setTerrain(Terrain* terrain);

    private:
        void damage();
        void spawn();

        static constexpr int M_DMG_TICK_MS = 200;
        static constexpr float M_SPAWN_TIME_MS = 1000;

        std::vector<Spawn> m_spawns;
        std::vector<std::tuple<DogType, int>> m_numSpawned;

        std::chrono::time_point<std::chrono::steady_clock> start;
        std::chrono::time_point<std::chrono::steady_clock> last;
        std::vector<Dog> m_dogs;
        unsigned int m_numKilled;

        Terrain* m_terrain;
};
