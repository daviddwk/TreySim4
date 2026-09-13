#pragma once

#include "dropParty.hpp"

#include <Eendgine/timer.hpp>

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

#include "items.hpp"
#include "terrain.hpp"

class DropParty {
        class Spawn {
            public:
                Spawn(
                    Tile position, Item::Type type,
                    std::optional<std::chrono::milliseconds> frequency)
                    : tile(position), type(type), item(std::nullopt),
                      timer(
                          frequency ? std::optional<Eend::Timer>(frequency->count() / 1000.0f)
                                    : std::nullopt) {};
                const Tile tile;
                const Item::Type type;
                std::optional<Item> item;
                std::optional<Eend::Timer> timer;
        };

    public:
        DropParty(std::filesystem::path parkPath);
        void update();
        void spawn(std::string tag);

    private:
        void spawnItem(Spawn& spawn);

        std::vector<Spawn> m_spawns;
        std::map<std::string, std::vector<Spawn>::size_type> m_spawnMap;
};
