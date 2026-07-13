#include "dropParty.hpp"

#include <Eendgine/fatalError.hpp>
#include <Eendgine/jsonUtils.hpp>
#include <Eendgine/random.hpp>

#include <glm/glm.hpp>
#include <json/json.h>
#include <print>

DropParty::DropParty(std::filesystem::path parkPath) {

    // generate the spawns
    std::filesystem::path metadataPath = parkPath / "generate/metadata.json";
    Json::Value rootJson = Eend::jsonLoadFile(metadataPath);

    Json::Value itemsJson = rootJson["Items"];
    if (!itemsJson.isArray()) {
        // Eend::fatalError("No Items array"); // TODO it should be okay not to have
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
    // actually spawn things TODO
}
