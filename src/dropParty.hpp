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
                Spawn(Tile position, Item::Type type, std::chrono::milliseconds frequency)
                    : tile(position), type(type), frequency(frequency), item(std::nullopt),
                      nextSpawn(frequency.count() / 1000.0f) {};
                const Tile tile;
                const Item::Type type;
                const std::chrono::milliseconds frequency;
                std::optional<Item> item;
                Eend::Timer nextSpawn;
        };

    public:
        DropParty(std::filesystem::path parkPath);
        void update();

    private:
        std::vector<Spawn> m_spawns;
};
