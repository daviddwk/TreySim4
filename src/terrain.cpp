#include "terrain.hpp"

#include "Eendgine/camera.hpp"
#include "collision.hpp"
#include "park.hpp"
#include "trey.hpp"

#include <Eendgine/board.hpp>
#include <Eendgine/doll.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/fatalError.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/jsonUtils.hpp>
#include <Eendgine/types.hpp>

#include <stb/stb_image.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

namespace Eend = Eendgine;

Json::Value jsonLoadFile(std::filesystem::path filePath);

const std::filesystem::path metadata_subpath("generate/metadata.json");
const std::filesystem::path heightmap_subpath("generate/heightMap.png");
const std::filesystem::path collisionmap_subpath("generate/collisionMap.png");

Terrain::Terrain(const std::filesystem::path path)
    : m_path(path), m_height(0), m_width(0), m_animation_speed(0), m_statue_id(0), m_spawn(0.0f) {
    // TODO this is possibly the worst code in this whole project

    std::filesystem::path metadata_path = path / metadata_subpath;
    Json::Value root_json = Eend::jsonLoadFile(metadata_path);

    m_scale = Eend::jsonVec3(root_json, "Scale", metadata_path);
    m_spawn = Eend::jsonVec2(root_json, "Spawn", metadata_path);

    // optional
    if (root_json["Pace"].isNumeric()) {
        m_animation_speed = Eend::jsonFloat(root_json, "Pace", metadata_path);
    }

    // optional
    if (root_json["Portals"].isArray()) {
        for (unsigned int portal_idx = 0; portal_idx < root_json["Portals"].size(); ++portal_idx) {
            m_portals.push_back(Terrain::portalFromJson(root_json["Portals"][portal_idx]));
        }
    }

    Terrain::loadHeightMap(m_height_map);

    // TODO i want to encapsulate this and make it clear that it's using the height map
    const std::filesystem::path collisionmap_path = m_path / collisionmap_subpath;
    Terrain::collisionFromMap(collisionmap_path, m_collision_rectangles);

    Terrain::loadEntities(root_json, m_boards, m_statues, m_dolls, m_height_map);

    // TODO make this conditional or a seperate util
    Terrain::createTerrainObj(m_height_map);
    m_statue_id = Eend::Entities::statues().insert(m_path);
}

Terrain::~Terrain() {
    Eend::Entities::statues().erase(m_statue_id);
    for (auto& board : m_boards)
        Eend::Entities::boards().erase(std::get<Eend::BoardId>(board));
    for (Eend::StatueId& statue : m_statues)
        Eend::Entities::statues().erase(statue);
    for (auto& doll : m_dolls)
        Eend::Entities::dolls().erase(std::get<Eend::DollId>(doll));
}

void Terrain::update() {
    static float cumulative = 0.0f;
    cumulative += Eend::FrameLimiter::get().deltaTime;

    auto animate_boards = [](std::vector<std::tuple<Eend::BoardId, float>>& paced_boards) {
        for (auto& board : paced_boards) {
            Eend::Board* board_ref =
                Eend::Entities::boards().getRef(std::get<Eend::BoardId>(board));
            if (std::get<float>(board) != 0) {
                board_ref->setStripIdx((size_t)(cumulative / std::get<float>(board)));
            }
        }
    };
    auto animate_dolls = [](std::vector<std::tuple<Eend::DollId, float>>& pacedDolls) {
        for (auto& doll : pacedDolls) {
            Eend::Doll* dollRef = Eend::Entities::dolls().getRef(std::get<Eend::DollId>(doll));
            float animScale = dollRef->getAnim();
            animScale += std::get<float>(doll) * Eend::FrameLimiter::get().deltaTime;
            dollRef->setAnim(animScale);
        }
    };

    animate_boards(m_boards);
    for (auto& [key, playground] : m_playgrounds) {
        animate_boards(playground.boards);
    }
    animate_dolls(m_dolls);
    for (auto& [key, playground] : m_playgrounds) {
        animate_dolls(playground.dolls);
    }

    if (m_animation_speed != 0.0f) {
        Eend::Entities::statues().getRef(m_statue_id)->setStripIdx(cumulative / m_animation_speed);
    }
}

bool Terrain::colliding(const Eend::Point2D point) {

    auto handle_portals = [point](std::vector<Portal>& portals) {
        for (auto& portal : portals) {
            if (pointOnRectangle(point, portal.collision)) {
                Park::get().setTerrain(portal.terrainPath);
            }
        }
    };
    handle_portals(m_portals);
    for (auto& [key, playground] : m_playgrounds) {
        handle_portals(playground.portals);
    }

    for (auto& rectangle : m_collision_rectangles) {
        if (pointOnRectangle(point, rectangle)) return true;
    }
    for (auto& [key, playground] : m_playgrounds) {
        for (auto& rectangle : playground.collision) {
            if (pointOnRectangle(point, rectangle)) return true;
        }
    }
    return false;
}

void Terrain::playgroundEnable(const std::string& playground_name) {
    assert(m_playgrounds.count(playground_name) == 0); // TODO idk but handle better

    std::filesystem::path metadata_path = m_path / metadata_subpath;
    Json::Value root_json = Eend::jsonLoadFile(metadata_path);

    m_playgrounds[playground_name];

    if (root_json["Playgrounds"].isObject()) {
        Json::Value pg_json = root_json["Playgrounds"][playground_name];
        assert(pg_json.isObject());
        Terrain::loadEntities(
            pg_json,
            m_playgrounds[playground_name].boards,
            m_playgrounds[playground_name].statues,
            m_playgrounds[playground_name].dolls,
            m_height_map);
        if (pg_json["Portals"].isArray()) {
            for (unsigned int portalIdx = 0; portalIdx < pg_json["Portals"].size(); ++portalIdx) {
                m_playgrounds[playground_name].portals.push_back(
                    portalFromJson(pg_json["Portals"][portalIdx]));
            }
        }
        if (pg_json["Collision"].isArray()) {
            for (unsigned int portalIdx = 0; portalIdx < pg_json["Collision"].size(); ++portalIdx) {
                collisionFromJson(
                    pg_json["Collision"][portalIdx],
                    m_playgrounds[playground_name].collision);
            }
        }
    }
}

void Terrain::playgroundDisable(const std::string& playground_name) {
    m_playgrounds.erase(playground_name);
}

bool Terrain::playgroundIsEnabled(const std::string& playground_name) {
    return m_playgrounds.count(playground_name) > 0;
}

void Terrain::playgroundToggle(const std::string& playground_name) {
    if (playgroundIsEnabled(playground_name)) {
        playgroundDisable(playground_name);
    } else {
        playgroundEnable(playground_name);
    }
}

Eend::Point Terrain::positionAtTile(const Tile tile) { return positionAtTile(tile, 0.0f); }

Eend::Point Terrain::positionAtTile(const Tile tile, const float height_offset) {
    return Eend::Point(
        (tile.x * m_scale.x) + (m_scale.x / 2.0),
        (tile.y * -m_scale.y) + (-m_scale.y / 2.0),
        height_offset + elevationAtPoint(Eend::Point2D(tile.x * m_scale.x, tile.y * -m_scale.y)));
}

float Terrain::elevationAtPoint(Eend::Point2D point) {
    // TODO I'm flipping the y around randomly here
    // be more explicit about pixel coords vs 3d coords

    //    top left +-------+ top right
    //             |\      |
    //             | \upper|
    //             |  \    |
    //             |   \   |
    //             |    \  |
    //             |lower\ |
    //             |      \|
    // bottom left +-------+ bottom right

    const float scaled_x = point.x / m_scale.x;
    const float scaled_y = point.y / m_scale.y;

    const bool outside_terrain = scaled_x < 0 || scaled_y > 0 ||
                                 scaled_x >= static_cast<float>(m_height_map[0].size() - 1) ||
                                 scaled_y <= static_cast<float>((-(float)m_height_map.size() + 1));
    // could make this an optional
    if (outside_terrain) {
        return 0.0f;
    }

    Eend::Triangle triangle(Eend::Point(0.0f), Eend::Point(0.0f), Eend::Point(0.0f));

    const float relativeX = scaled_x - (float)floor(scaled_x);
    const float relativeY = -scaled_y - (float)floor(-scaled_y);
    const bool upperTri = (relativeX - relativeY) > 0;

    const int topLeftXIdx = floor(scaled_x);
    const int topLeftYIdx = floor(-scaled_y);

    const int bottomRightXIdx = floor(scaled_x + 1);
    const int bottomRightYIdx = floor(-scaled_y + 1);

    const Eend::Point topLeftPoint = Eend::Point(
        static_cast<float>(topLeftXIdx) * m_scale.x,
        static_cast<float>(topLeftYIdx) * -m_scale.y,
        m_height_map[topLeftYIdx][topLeftXIdx]);

    const Eend::Point bottomRightPoint = Eend::Point(
        static_cast<float>(bottomRightXIdx) * m_scale.x,
        static_cast<float>(bottomRightYIdx) * -m_scale.y,
        m_height_map[bottomRightYIdx][bottomRightXIdx]);

    if (upperTri) {

        const int topRightXIdx = floor(scaled_x + 1);
        const int topRightYIdx = floor(-scaled_y);

        const Eend::Point topRightPoint = Eend::Point(
            static_cast<float>(topRightXIdx * m_scale.x),
            static_cast<float>(topRightYIdx * -m_scale.y),
            m_height_map[topRightYIdx][topRightXIdx]);

        triangle = Eend::Triangle(topLeftPoint, topRightPoint, bottomRightPoint);

    } else {

        const int bottomLeftXIdx = floor(scaled_x);
        const int bottomLeftYIdx = floor(-scaled_y + 1);
        const Eend::Point bottomLeftPoint = Eend::Point(
            static_cast<float>(bottomLeftXIdx * m_scale.x),
            static_cast<float>(bottomLeftYIdx * -m_scale.y),
            m_height_map[bottomLeftYIdx][bottomLeftXIdx]);

        triangle = Eend::Triangle(topLeftPoint, bottomLeftPoint, bottomRightPoint);
    }
    return pointHeightOnTri(triangle, point);
}

void Terrain::loadHeightMap(std::vector<std::vector<float>>& heightMap) {
    // create height map
    const std::filesystem::path heightMapPath = m_path / heightmap_subpath;
    int channels = 0;
    unsigned char* imageData = stbi_load(heightMapPath.c_str(), &m_width, &m_height, &channels, 0);
    constexpr float IMAGE_DATA_MAX_VALUE = 256.0f;

    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + heightMapPath.generic_string() + " for terrain generation");
    }

    for (int h = 0; h < (m_height - 1); ++h) {
        // handling edge cases looks horrible ugly, i'll admit
        heightMap.emplace_back();
        if (h == 0) heightMap.emplace_back();
        if (h == (m_height - 2)) heightMap.emplace_back();
        for (int w = 0; w < (m_width - 1); ++w) {
            unsigned int averageHeight = 0;

            const size_t currentIdx = w + (h * m_width);
            const size_t rightIdx = (w + 1) + (h * m_width);
            const size_t downIdx = w + ((h + 1) * m_width);
            const size_t rightDownIdx = downIdx + 1;

            const bool isEdgeXLeft = (w == 0);
            const bool isEdgeXRight = (w == (m_width - 2));
            const bool isEdgeYTop = (h == 0);
            const bool isEdgeYBottom = (h == (m_height - 2));

            const unsigned char currentHeight = imageData[currentIdx];
            const unsigned char rightHeight = imageData[rightIdx];
            const unsigned char downHeight = imageData[downIdx];
            const unsigned char rightDownHeight = imageData[rightDownIdx];

            auto scaleHeight = [&](unsigned int height) {
                return (height / IMAGE_DATA_MAX_VALUE) * m_scale.z;
            };

            if (isEdgeYTop) {
                if (isEdgeXLeft) {
                    averageHeight = currentHeight;
                    heightMap[h].push_back(scaleHeight(averageHeight));
                }

                averageHeight = (currentHeight + rightHeight) / 2;
                heightMap[h].push_back(scaleHeight(averageHeight));

                if (isEdgeXRight) {
                    averageHeight = currentHeight;
                    heightMap[h].push_back(scaleHeight(averageHeight));
                }
            }
            if (isEdgeYBottom) {
                if (isEdgeXLeft) {
                    averageHeight = downHeight;
                    heightMap[h + 2].push_back(scaleHeight(averageHeight));
                }

                averageHeight = (downHeight + rightDownHeight) / 2;
                heightMap[h + 2].push_back(scaleHeight(averageHeight));

                if (isEdgeXRight) {
                    averageHeight = downHeight;
                    heightMap[h + 2].push_back(scaleHeight(averageHeight));
                }
            }
            if (isEdgeXLeft) {
                averageHeight = (currentHeight + downHeight) / 2;
                heightMap[h + 1].push_back(scaleHeight(averageHeight));
            }

            averageHeight = ((currentHeight + rightHeight + downHeight + rightDownHeight) / 4);
            heightMap[h + 1].push_back(scaleHeight(averageHeight));

            if (isEdgeXRight) {
                averageHeight = (currentHeight + downHeight) / 2;
                heightMap[h + 1].push_back(scaleHeight(averageHeight));
            }
        }
    }
    stbi_image_free(imageData);
    // verify height map
    if ((int)heightMap.size() == 0) {
        Eend::fatalError("height map is empty");
    }
    if ((int)heightMap.size() != (m_height + 1)) {
        Eend::fatalError("height map unexpected height");
    }
    for (auto& line : heightMap) {
        if ((int)line.size() != (m_width + 1)) {
            Eend::fatalError("height map unexpected line width");
        }
    }
}

void Terrain::loadEntities(
    Json::Value rootJson, std::vector<std::tuple<Eend::BoardId, float>>& boards,
    std::vector<Eend::StatueId>& statues, std::vector<std::tuple<Eend::DollId, float>> dolls,
    const std::vector<std::vector<float>> heightMap) {

    // TODO i want to encapsulate this and make it clear that it's using the height map

    if (rootJson["Boards"].isArray()) {
        for (unsigned int boardIdx = 0; boardIdx < rootJson["Boards"].size(); ++boardIdx) {
            boards.push_back(boardFromJson(rootJson["Boards"][boardIdx]));
        }
    }
    if (rootJson["Statues"].isArray()) {
        for (unsigned int statueIdx = 0; statueIdx < rootJson["Statues"].size(); ++statueIdx) {
            statues.push_back(statueFromJson(rootJson["Statues"][statueIdx]));
        }
    }
    if (rootJson["Dolls"].isArray()) {
        for (unsigned int dollIdx = 0; dollIdx < rootJson["Dolls"].size(); ++dollIdx) {
            dolls.push_back(dollFromJson(rootJson["Dolls"][dollIdx]));
        }
    }
}

void Terrain::createTerrainObj(const std::vector<std::vector<float>>& heightMap) {
    std::filesystem::path heightMapPath = m_path / heightmap_subpath;
    std::string fileName = heightMapPath.stem().string();

    std::filesystem::path objPath = m_path / m_path.filename().string().append(".obj");
    std::ofstream objFile;
    objFile.open(objPath);
    if (!objFile.is_open())
        Eend::fatalError(std::format("unable to open file {}\n", objPath.generic_string()));

    objFile << "# TreySim4" << std::endl;
    objFile << "mtllib " << fileName << ".mtl" << std::endl;
    objFile << "o " << fileName << std::endl;

    float xPos = 0.0f;
    float yPos = 0.0f;
    // vert positions
    for (auto& line : heightMap) {
        for (const float& height : line) {
            // x and y Pos also UV map?
            objFile << "v " << xPos * m_scale.x << " " << yPos * m_scale.y << " " << height
                    << std::endl; // height << std::endl;
            xPos += 1.0f;
        }
        xPos = 0.0f;
        yPos -= 1.0f;
    }
    // uv mapping
    for (size_t lineIdx = 0; lineIdx < heightMap.size(); ++lineIdx) {
        float heightRatio = 1.0f - ((float)lineIdx / (float)(heightMap.size() - 1.0f));
        for (unsigned int eleIdx = 0; eleIdx < heightMap[lineIdx].size(); ++eleIdx) {
            float widthRatio = (float)eleIdx / (float)(heightMap[lineIdx].size() - 1.0f);
            objFile << "vt " << widthRatio << " " << heightRatio << std::endl;
        }
    }

    objFile << "s 0" << std::endl; // I have no idea
    objFile << "usemtl Material" << std::endl;
    // tris by vert index
    for (size_t lineIdx = 0; lineIdx < (heightMap.size() - 1); ++lineIdx) {
        for (unsigned int eleIdx = 0; eleIdx < (heightMap[lineIdx].size() - 1); ++eleIdx) {
            // I think this is 1 indexed :c
            unsigned int flatIdx = eleIdx + (lineIdx * heightMap[lineIdx].size()) + 1;
            unsigned int rightIdx = flatIdx + 1;
            unsigned int downIdx = flatIdx + heightMap[lineIdx].size();
            unsigned int rightDownIdx = downIdx + 1;

            objFile << "f " << rightIdx << '/' << rightIdx << '/' << rightIdx << ' ';
            objFile << flatIdx << '/' << flatIdx << '/' << flatIdx << ' ';
            objFile << rightDownIdx << '/' << rightDownIdx << '/' << rightDownIdx << std::endl;

            objFile << "f " << flatIdx << '/' << flatIdx << '/' << flatIdx << ' ';
            objFile << downIdx << '/' << downIdx << '/' << downIdx << ' ';
            objFile << rightDownIdx << '/' << rightDownIdx << '/' << rightDownIdx << std::endl;
        }
    }
    objFile.close();
}

std::tuple<Eend::BoardId, float> Terrain::boardFromJson(Json::Value boardJson) {
    std::filesystem::path metadataPath = m_path / metadata_subpath;
    std::filesystem::path boardPath = m_path / Eend::jsonString(boardJson, "path", metadataPath);
    Eend::BoardId id = Eend::Entities::boards().insert(boardPath);
    Eend::Board* boardRef = Eend::Entities::boards().getRef(id);
    float pace = 0;
    if (boardJson.isMember("pace")) pace = Eend::jsonFloat(boardJson, "pace", metadataPath);

    glm::vec3 position = Eend::jsonVec3(boardJson, "position", metadataPath);
    boardRef->setPosition(positionAtTile(Tile(position.x, position.y), position.z));
    boardRef->setRotation(Eend::jsonFloat(boardJson, "rotation", metadataPath));
    boardRef->setScale(Eend::jsonVec3(boardJson, "scale", metadataPath));
    return std::tie(id, pace);
}

Eend::StatueId Terrain::statueFromJson(Json::Value statueJson) {
    std::filesystem::path metadataPath = m_path / metadata_subpath;
    std::filesystem::path statuePath = m_path / Eend::jsonString(statueJson, "path", metadataPath);
    Eend::StatueId id = Eend::Entities::statues().insert(statuePath);
    Eend::Statue* statueRef = Eend::Entities::statues().getRef(id);

    glm::vec3 position = Eend::jsonVec3(statueJson, "position", metadataPath);
    statueRef->setPosition(positionAtTile(Tile(position.x, position.y), position.z));
    statueRef->setRotation(Eend::jsonVec3(statueJson, "rotation", metadataPath));
    statueRef->setScale(Eend::jsonVec3(statueJson, "scale", metadataPath));
    return id;
}

std::tuple<Eend::DollId, float> Terrain::dollFromJson(Json::Value dollJson) {
    std::filesystem::path metadataPath = m_path / metadata_subpath;
    std::filesystem::path dollPath = m_path / Eend::jsonString(dollJson, "path", metadataPath);
    Eend::DollId id = Eend::Entities::dolls().insert(dollPath);
    Eend::Doll* dollRef = Eend::Entities::dolls().getRef(id);
    float pace = 0;
    if (dollJson.isMember("pace")) pace = Eend::jsonFloat(dollJson, "pace", metadataPath);

    glm::vec3 position = Eend::jsonVec3(dollJson, "position", metadataPath);
    dollRef->setPosition(positionAtTile(Tile(position.x, position.y), position.z));
    dollRef->setRotation(Eend::jsonVec3(dollJson, "rotation", metadataPath));
    dollRef->setScale(Eend::jsonVec3(dollJson, "scale", metadataPath));
    if (dollJson.isMember("animation"))
        dollRef->setAnimation(Eend::jsonString(dollJson, "animation", metadataPath));
    return std::tie(id, pace);
}

Portal Terrain::portalFromJson(Json::Value portalJson) {
    std::filesystem::path metadataPath = m_path / metadata_subpath;
    Tile tilePosition = Eend::jsonVec2(portalJson, "position", metadataPath);
    TileScale tileScale = Eend::jsonVec2(portalJson, "scale", metadataPath);

    Eend::Point2D position = glm::vec2(tilePosition.x * m_scale.x, -tilePosition.y * m_scale.y);
    Eend::Vector2D toCorner = glm::vec2(tileScale.x * m_scale.x, tileScale.y * m_scale.y);

    std::filesystem::path terrainPath = Eend::jsonString(portalJson, "path", metadataPath);

    return Portal(position, toCorner, terrainPath);
}

void Terrain::collisionFromJson(
    Json::Value collisionJson, std::vector<Eend::Rectangle>& collisionVec) {
    std::filesystem::path metadataPath = m_path / metadata_subpath;
    std::filesystem::path collisionMap;

    if (collisionJson.isMember("map")) {
        std::string collisionName = Eend::jsonString(collisionJson, "map", metadataPath);
        collisionMap = m_path / "collision" / (collisionName + ".png");
    }
    collisionFromMap(collisionMap, collisionVec);
}

// must call after m_scale is determined
void Terrain::collisionFromMap(
    const std::filesystem::path collisionMap, std::vector<Eend::Rectangle>& collisionVec) {

    int collisionHeight = 0;
    int collisionWidth = 0;
    int channels = 1;
    unsigned char* imageData =
        stbi_load(collisionMap.c_str(), &collisionWidth, &collisionHeight, &channels, 0);
    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + collisionMap.generic_string() + " for terrain generation");
    }
    for (int h = 0; h < collisionHeight; ++h) {
        for (int w = 0; w < collisionWidth; ++w) {
            const size_t currentIdx = w + (h * collisionWidth);
            if (imageData[currentIdx] == 0) {

                const float upperLeftX = (w * m_scale.x);
                const float upperLeftY = -((h + 1) * m_scale.y);
                const float lowerRightX = ((w + 1) * m_scale.x);
                const float lowerRightY = -(h * m_scale.y);

                const Eend::Point2D upperLeft(upperLeftX, upperLeftY);
                const Eend::Point2D lowerRight(lowerRightX, lowerRightY);

                collisionVec.emplace_back(upperLeft, lowerRight);
            }
        }
    }
    stbi_image_free(imageData);
}
