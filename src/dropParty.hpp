#pragma once

#include "dropParty.hpp"

#include <chrono>
#include <filesystem>

#include "items.hpp"
#include "terrain.hpp"

class DropParty {
        class Spawn {
            public:
                Spawn(Tile position, Item::Type type, std::chrono::milliseconds frequency)
                    : tile(position), type(type), empty(true), frequency(frequency) {};
                Tile tile;
                Item::Type type;
                bool empty;
                std::chrono::milliseconds frequency;
                std::chrono::time_point<std::chrono::steady_clock> nextSpawn;
        };

    public:
        DropParty(std::filesystem::path parkPath);
        void update();

    private:
        std::vector<Spawn> m_spawns;
        std::vector<Item> m_items;
};
