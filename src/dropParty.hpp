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
                    Tile position, std::filesystem::path texture, Item::Type type,
                    Item::Traits traits, std::optional<std::chrono::milliseconds> frequency)
                    : tile(position), texture(texture), type(type), traits(traits),
                      item(std::nullopt),
                      timer(
                          frequency ? std::optional<Eend::Timer>(frequency->count() / 1000.0f)
                                    : std::nullopt) {};
                const Tile tile;
                std::filesystem::path texture;
                const Item::Type type;
                const Item::Traits traits;
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
