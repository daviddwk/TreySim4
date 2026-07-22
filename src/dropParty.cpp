#include "dropParty.hpp"

#include "trey.hpp"

#include <Eendgine/fatalError.hpp>
#include <Eendgine/jsonUtils.hpp>
#include <Eendgine/random.hpp>

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
    if (!itemsJson.isNull() && !itemsJson.isArray()) {
        Eend::fatalError("Items exsists but isn't an array"); // TODO it should be okay not to have
    }
    for (int itemSpawnIdx = 0; itemSpawnIdx < itemsJson.size(); ++itemSpawnIdx) {
        Json::Value itemJson = itemsJson[itemSpawnIdx];
        Json::Value typeJson = itemsJson[itemSpawnIdx]["type"];
        Json::Value frequencyJson = itemsJson[itemSpawnIdx]["frequency"];

        m_spawns.emplace_back(
            Eend::jsonVec2(itemsJson[itemSpawnIdx], "position", metadataPath),
            Item::typeFromString(Eend::jsonString(itemsJson[itemSpawnIdx], "type", metadataPath)),
            std::chrono::milliseconds(
                Eend::jsonInt(itemsJson[itemSpawnIdx], "frequency", metadataPath)));
    }
}

void DropParty::update() {
    // spawn things
    for (Spawn& spawn : m_spawns) {
        if (!spawn.item) {
            Eend::Point2D position2d = Park::get().positionAtTile(spawn.tile);
            Eend::Point position = Eend::Point(
                position2d.x,
                position2d.y,
                Park::get().heightAtPoint(position2d) + ITEM_HEIGHT_OFFSET);
            spawn.item = Item(spawn.type, position);
        }
    }
    // get item
    for (Spawn& spawn : m_spawns) {
        if (spawn.item) {
            const Eend::Point treyPosition = Trey::get().getPosition();
            const float treyRadius = Trey::get().getRadius();
            const float distance = glm::length(spawn.item->getPosition() - treyPosition);
            if (distance < treyRadius) {
                // give trey power
                Trey::get().setItem(spawn.item->getType());
                spawn.item = std::nullopt;
            }
        }
    }
}
