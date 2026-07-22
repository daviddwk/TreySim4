#pragma once

#include "dropParty.hpp"

#include <chrono>
#include <filesystem>
#include <optional>

#include "items.hpp"
#include "terrain.hpp"

class DropParty {
        class Spawn {
            public:
                Spawn(Tile position, Item::Type type, std::chrono::milliseconds frequency)
                    : tile(position), type(type), frequency(frequency), item(std::nullopt),
                      nextSpawn(std::chrono::steady_clock::now() + frequency) {};
                const Tile tile;
                const Item::Type type;
                const std::chrono::milliseconds frequency;
                std::optional<Item> item;
                std::chrono::time_point<std::chrono::steady_clock> nextSpawn;
        };

    public:
        DropParty(std::filesystem::path parkPath);
        void update();

    private:
        std::vector<Spawn> m_spawns;
};
