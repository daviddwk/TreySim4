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
                    : tile(position), type(type), item(std::nullopt), frequency(frequency) {};
                Tile tile;
                Item::Type type;
                std::optional<Item> item;
                std::chrono::milliseconds frequency;
                std::chrono::time_point<std::chrono::steady_clock> nextSpawn;
        };

    public:
        DropParty(std::filesystem::path parkPath);
        void update();

    private:
        std::vector<Spawn> m_spawns;
};
