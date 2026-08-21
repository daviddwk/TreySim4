#pragma once

#include "dog.hpp"
#include "terrain.hpp"

#include <Eendgine/timer.hpp>

#include <chrono>
#include <filesystem>
#include <json/json.h>
#include <memory>
#include <optional>

class PuppyMill {
        class Spawn {
            public:
                Spawn(Tile tile, Dog::Type dogType, std::chrono::milliseconds frequency)
                    : tile(tile), dogType(dogType), frequency(frequency),
                      nextSpawn(frequency.count() / 1000.0f) {};
                Tile tile;
                Dog::Type dogType;
                std::chrono::milliseconds frequency;

                Eend::Timer nextSpawn;
                int dogsSpawned = 0;
        };
        class Wave {
            public:
                Wave(std::optional<std::chrono::seconds> duration) : end(std::nullopt) {
                    if (duration.has_value()) {
                        end = static_cast<float>(duration.value().count());
                    }
                };
                std::vector<Spawn> spawns;
                std::optional<Eend::Timer> end;
        };

    public:
        PuppyMill(std::weak_ptr<Terrain> terrain, std::filesystem::path parkPath);

        // returns true if wave overflow
        bool setWaveIdx(std::vector<Dog>::size_type waveIdx);
        // returns true if end of waves
        bool nextWave();

        void update();

        unsigned int getNumKilled();
        int getNumDogs();
        int getWaveIdx();

    private:
        void damage();
        void spawn();

        void wavesFromJson(const std::filesystem::path& parkPath, std::vector<Wave>& spawnWaves);
        void processSpawnJson(const Json::Value& dogSpawnJson, std::vector<Spawn>& spawns);

        static constexpr int M_DMG_TICK_MS = 200;
        static constexpr float M_SPAWN_TIME_MS = 1000;

        std::vector<Wave> m_spawnWaves;
        std::vector<std::tuple<Dog::Type, int>> m_numSpawned;

        std::chrono::time_point<std::chrono::steady_clock> m_start;
        std::chrono::time_point<std::chrono::steady_clock> m_last;
        std::vector<Dog> m_dogs;
        unsigned int m_numKilled;

        std::weak_ptr<Terrain> m_terrain;
        std::vector<Dog>::size_type m_waveIdx;
};
