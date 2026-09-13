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

        Item::Type item = Item::typeFromString(Eend::jsonString(itemJson, "type", metadataPath));

        std::optional<std::chrono::milliseconds> frequency = std::nullopt;
        if (itemJson["frequency"].isNumeric()) {
            frequency =
                std::chrono::milliseconds(Eend::jsonInt(itemJson, "frequency", metadataPath));
        }

        if (itemJson["tag"].isString()) {
            std::string tag = Eend::jsonString(itemJson, "tag", metadataPath);
            m_spawnMap[tag] = m_spawns.size();
        }

        m_spawns.emplace_back(position, item, frequency);
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
            const float treyRadius = Trey::get().getRadius();
            const float distance = glm::length(spawn.item->getPosition() - treyPosition);
            if (distance < treyRadius) {
                Trey::get().setItem(spawn.item->getType());
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
    spawn.item = Item(spawn.type, position);
}
