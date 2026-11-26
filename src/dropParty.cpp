#include "dropParty.hpp"

#include "trey.hpp"

#include <Eendgine/fatalError.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/jsonUtils.hpp>
#include <Eendgine/random.hpp>

#include <assimp/metadata.h>
#include <chrono>
#include <glm/glm.hpp>
#include <json/json.h>
#include <print>

#include "park.hpp"

constexpr float ITEM_HEIGHT_OFFSET = 5.0f;
constexpr float ITEM_RADIUS = 5.5f;

DropParty::DropParty(std::filesystem::path parkPath) {

    // generate the spawns
    std::filesystem::path metadataPath = parkPath / "generate/metadata.json";
    Json::Value rootJson = Eend::jsonLoadFile(metadataPath);

    Json::Value itemsJson = rootJson["Items"];
    if (itemsJson.isNull()) {
        return;
    } else if (!itemsJson.isArray()) {
        Eend::fatalError("Items exsists but isn't an array");
    }
    for (int itemSpawnIdx = 0; itemSpawnIdx < itemsJson.size(); ++itemSpawnIdx) {

        Json::Value itemJson = itemsJson[itemSpawnIdx];

        Tile position = Eend::jsonVec2(itemJson, "position", metadataPath);

        std::filesystem::path texture = Eend::jsonString(itemJson, "texture", metadataPath);

        // Item::Type item = Item::typeFromString(Eend::jsonString(itemJson, "type", metadataPath));
        // should resolve strings to types here TODO

        std::optional<std::chrono::milliseconds> frequency = std::nullopt;
        if (itemJson["frequency"].isNumeric()) {
            frequency =
                std::chrono::milliseconds(Eend::jsonInt(itemJson, "frequency", metadataPath));
        }

        if (itemJson["tag"].isString()) {
            std::string tag = Eend::jsonString(itemJson, "tag", metadataPath);
            m_spawnMap[tag] = m_spawns.size();
        }

        Item::Type item_type;
        std::string type_string = Eend::jsonString(itemJson, "type", metadataPath);
        if (type_string == "circle") {
            item_type = Item::Type::Circle;
        } else if (type_string == "throw") {
            item_type = Item::Type::Throw;
        } else if (type_string == "kick") {
            item_type = Item::Type::Kick;
        } else {
            item_type = Item::Type::Circle;
            Eend::fatalError("item type not supported: " + type_string);
        }

        Json::Value traitsJson = itemJson["traits"];
        Item::Traits traits;
        switch (item_type) {
        case Item::Type::Circle:
            traits.boot_circle.boot_radius =
                Eend::jsonFloat(traitsJson, "boot_radius", metadataPath);
            traits.boot_circle.circle_radius =
                Eend::jsonFloat(traitsJson, "circle_radius", metadataPath);
            traits.boot_circle.speed = Eend::jsonFloat(traitsJson, "speed", metadataPath);
            break;
        case Item::Type::Throw:
            traits.boot_throw.boot_radius =
                Eend::jsonFloat(traitsJson, "boot_radius", metadataPath);
            traits.boot_throw.throw_distance =
                Eend::jsonFloat(traitsJson, "throw_distance", metadataPath);
            traits.boot_throw.throw_frequency =
                Eend::jsonFloat(traitsJson, "throw_frequency", metadataPath);
            traits.boot_throw.throw_speed =
                Eend::jsonFloat(traitsJson, "throw_speed", metadataPath);
            break;
        case Item::Type::Kick:
            break;
        }

        m_spawns.emplace_back(position, texture, item_type, traits, frequency); // TODO bring back
    }
}

void DropParty::update() {
    // spawn things
    float dt = Eend::FrameLimiter::get().deltaTime;
    for (Spawn& spawn : m_spawns) {
        if (spawn.timer) {                                // if it's an auto spawner
            if (!spawn.item && spawn.timer->update(dt)) { // if its empty and it's time
                spawnItem(spawn);
            }
        }
    }
    // get item
    for (Spawn& spawn : m_spawns) {
        if (spawn.item) {
            const Eend::Point treyPosition = Trey::get().getPosition();
            const float distance = glm::length(spawn.item->getPosition() - treyPosition);
            if (distance < ITEM_RADIUS) {
                switch (spawn.type) {
                case Item::Type::Circle:
                    Trey::get().pickupItem(spawn.item->getBootCircleTraits());
                    break;
                case Item::Type::Throw:
                    Trey::get().pickupItem(spawn.item->getBootThrowTraits());
                    break;
                default:
                    Eend::fatalError("item type not handled");
                    break;
                }
                spawn.item = std::nullopt;
            }
        }
    }
}

void DropParty::spawn(std::string tag) {
    assert(m_spawnMap.contains(tag));
    Spawn& spawn = m_spawns[m_spawnMap[tag]];
    if (!spawn.item) spawnItem(m_spawns[m_spawnMap[tag]]);
}

void DropParty::spawnItem(Spawn& spawn) {
    if (spawn.timer) spawn.timer->reset();
    Eend::Point2D position2d = Park::get().positionAtTile(spawn.tile);
    Eend::Point position = Eend::Point(
        position2d.x,
        position2d.y,
        Park::get().elevationAtPoint(position2d) + ITEM_HEIGHT_OFFSET);
    switch (spawn.type) {
    case Item::Type::Circle:
        spawn.item = Item(position, spawn.texture, spawn.traits.boot_circle);
        break;
    case Item::Type::Throw:
        spawn.item = Item(position, spawn.texture, spawn.traits.boot_throw);
        break;
    default:
        Eend::fatalError("item type not handled");
        break;
    }
}
