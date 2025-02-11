#include "stage.hpp"
#include "terrain.hpp"
#include <Eendgine/fatalError.hpp>
#include <filesystem>
#include <fstream>
#include <json/json.h>

namespace Eend = Eendgine;

Stage::Stage(std::string stagePath) {
    Json::Value root;
    std::filesystem::path metadataPath(std::filesystem::path("stages") /
                                       std::filesystem::path(stagePath) / std::filesystem::path());
    std::ifstream metadata(metadataPath);
    if (!metadata.is_open()) {
        Eend::fatalError("could not open: " + metadataPath.string());
    }
    try {
        metadata >> root;
    } catch (...) {
        Eend::fatalError("improper json: " + metadataPath.string());
    }

    const Json::Value boards = root["boards"];
    const Json::Value statues = root["statues"];
    const Json::Value dolls = root["dolls"];

    if (boards.isArray()) {
        for (unsigned int i = 0; i < boards.size(); ++i) {
            if (!boards[i]["name"].isString()) {
                Eend::fatalError("name");
            }
            if (!boards[i]["position"]["x"].isNumeric()) {
                Eend::fatalError("pos x");
            }
            if (!boards[i]["position"]["y"].isNumeric()) {
                Eend::fatalError("pos y");
            }
            if (!boards[i]["scale"]["x"].isNumeric()) {
                Eend::fatalError("scale x");
            }
            if (!boards[i]["scale"]["y"].isNumeric()) {
                Eend::fatalError("scale y");
            }
            const std::string name = boards[i]["name"].asString();
            Eend::Point2D position(
                boards[i]["position"]["x"].asFloat(), boards[i]["position"]["y"].asFloat());
            Eend::Scale2D scale(
                boards[i]["scale"]["x"].asFloat(), boards[i]["scale"]["y"].asFloat());
            const Eend::BoardId id =
                Eend::Entities::BoardBatch::insert(std::filesystem::path("resources/" + name));
            _boardIds.push_back(id);
            Eend::Board& boardRef = Eend::Entities::BoardBatch::getRef(id);
            // we need to set this at the height of the terrain here actuallieeee
            boardRef.setPosition(Eend::Point(position.x, position.y, 0.0f));
            boardRef.setScale(scale);
        }
    }
}

Stage::~Stage() {}
