#pragma once

#include "dropParty.hpp"

#include <Eendgine/timer.hpp>

#include <chrono>
#include <filesystem>
#include <optional>

#include "items.hpp"
#include "terrain.hpp"

class DropParty {
        class Spawn {
            public:
                Spawn(
                    Tile position, Item::Type type,
                    std::optional<std::chrono::milliseconds> frequency)
                    : tile(position), type(type), frequency(frequency), item(std::nullopt),
                      nextSpawn(
                          frequency ? std::optional<Eend::Timer>(frequency->count() / 1000.0f)
                                    : std::nullopt) {};
                const Tile tile;
                const Item::Type type;
                // TODO combine frequency and next spawn
                std::optional<const std::chrono::milliseconds> frequency;
                std::optional<Item> item;
                std::optional<Eend::Timer> nextSpawn;
        };

    public:
        DropParty(std::filesystem::path parkPath);
        void update();
        void spawn(int idx);

    private:
        std::vector<Spawn> m_spawns;
};
