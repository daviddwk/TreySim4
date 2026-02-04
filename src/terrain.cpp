#include "terrain.hpp"

#include "collision.hpp"
#include "duck.hpp"

#include <Eendgine/board.hpp>
#include <Eendgine/doll.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/fatalError.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/types.hpp>

#include <json/json.h>
#include <stb/stb_image.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

namespace Eend = Eendgine;

Terrain::Terrain(const std::filesystem::path path, Eend::Scale scale)
    : m_height(0), m_width(0), m_statueId(0), m_scale(scale), m_spawnTileX(0.0f),
      m_spawnTileY(0.0f) {
    // height map from image
    std::filesystem::path pngHeightMap = "resources" / path / "heightMap.png";
    std::filesystem::path pngCollisionMap = "resources" / path / "collisionMap.png";
    int collisionHeight = 0;
    int collisionWidth = 0;
    int channels = 0;

    unsigned char* imageData = stbi_load(pngHeightMap.c_str(), &m_width, &m_height, &channels, 0);
    float IMAGE_DATA_MAX_VALUE = 256.0f;

    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + pngHeightMap.generic_string() + " for terrain generation");
    }

    for (int h = 0; h < (m_height - 1); ++h) {
        // handling edge cases looks horrible ugly, i'll admit
        m_heightMap.emplace_back();
        if (h == 0) m_heightMap.emplace_back();
        if (h == (m_height - 2)) m_heightMap.emplace_back();
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
                    m_heightMap[h].push_back(scaleHeight(averageHeight));
                }

                averageHeight = (currentHeight + rightHeight) / 2;
                m_heightMap[h].push_back(scaleHeight(averageHeight));

                if (isEdgeXRight) {
                    averageHeight = currentHeight;
                    m_heightMap[h].push_back(scaleHeight(averageHeight));
                }
            }
            if (isEdgeYBottom) {
                if (isEdgeXLeft) {
                    averageHeight = downHeight;
                    m_heightMap[h + 2].push_back(scaleHeight(averageHeight));
                }

                averageHeight = (downHeight + rightDownHeight) / 2;
                m_heightMap[h + 2].push_back(scaleHeight(averageHeight));

                if (isEdgeXRight) {
                    averageHeight = downHeight;
                    m_heightMap[h + 2].push_back(scaleHeight(averageHeight));
                }
            }
            if (isEdgeXLeft) {
                averageHeight = (currentHeight + downHeight) / 2;
                m_heightMap[h + 1].push_back(scaleHeight(averageHeight));
            }

            averageHeight = ((currentHeight + rightHeight + downHeight + rightDownHeight) / 4);
            m_heightMap[h + 1].push_back(scaleHeight(averageHeight));

            if (isEdgeXRight) {
                averageHeight = (currentHeight + downHeight) / 2;
                m_heightMap[h + 1].push_back(scaleHeight(averageHeight));
            }
        }
    }
    stbi_image_free(imageData);
    channels = 1;
    imageData = stbi_load(pngCollisionMap.c_str(), &collisionWidth, &collisionHeight, &channels, 0);
    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + pngCollisionMap.generic_string() + " for terrain generation");
    }
    for (int h = 0; h < collisionHeight; ++h) {
        for (int w = 0; w < collisionWidth; ++w) {
            const size_t currentIdx = w + (h * collisionWidth);
            if (imageData[currentIdx] == 0) {

                const float upperLeftX = (w * m_scale.x) + 1;
                const float upperLeftY = -(((h + 1) * m_scale.y) + 1);
                const float lowerRightX = ((w + 1) * m_scale.x) + 1;
                const float lowerRightY = -((h * m_scale.y) + 1);

                const Eend::Point2D upperLeft(upperLeftX, upperLeftY);
                const Eend::Point2D lowerRight(lowerRightX, lowerRightY);

                m_collisionRectangles.emplace_back(upperLeft, lowerRight);
            }
        }
    }
    stbi_image_free(imageData);
    //
    Json::Value rootJson;
    std::filesystem::path metadataPath = "resources" / path / "metadata.json";
    std::ifstream metadata(metadataPath);
    if (!metadata.is_open()) {
        Eend::fatalError("could not open: " + metadataPath.string());
    }
    try {
        metadata >> rootJson;
    } catch (...) {
        Eend::fatalError("improper json: " + metadataPath.string());
    }

    if (rootJson["Spawn"].isArray()) {
        m_spawnTileX = rootJson["Spawn"][0].asFloat();
        m_spawnTileY = rootJson["Spawn"][1].asFloat();
    }

    if (rootJson["Boards"].isArray()) {
        for (unsigned int boardIdx = 0; boardIdx < rootJson["Boards"].size(); ++boardIdx) {
            Json::Value boardJson = rootJson["Boards"][boardIdx];
            Eend::BoardId id = Eend::Entities::boards().insert(boardJson["path"].asString());
            Eend::Board* boardRef = Eend::Entities::boards().getRef(id);
            float pace = boardJson["pace"].asFloat(); // should be 0 if not there?
            m_boards.push_back(std::tie(id, pace));

            float tileXIdx = boardJson["position"][0].asFloat();
            float tileYIdx = boardJson["position"][1].asFloat();
            float heightOffset = boardJson["position"][2].asFloat();

            boardRef->setPosition(positionAtTile(tileXIdx, tileYIdx, heightOffset));
            boardRef->setRotation(boardJson["rotation"].asFloat());
            boardRef->setScale(
                Eend::Scale2D(boardJson["scale"][0].asFloat(), boardJson["scale"][1].asFloat()));
        }
    }
    if (rootJson["Statues"].isArray()) {
        for (unsigned int statueIdx = 0; statueIdx < rootJson["Statues"].size(); ++statueIdx) {
            Json::Value statueJson = rootJson["Statues"][statueIdx];
            Eend::StatueId id = Eend::Entities::statues().insert(statueJson["path"].asString());
            Eend::Statue* statueRef = Eend::Entities::statues().getRef(id);
            m_statues.push_back(id);

            float tileXIdx = statueJson["position"][0].asFloat();
            float tileYIdx = statueJson["position"][1].asFloat();
            float heightOffset = statueJson["position"][2].asFloat();

            statueRef->setPosition(positionAtTile(tileXIdx, tileYIdx, heightOffset));
            statueRef->setRotation(
                statueJson["rotation"][0].asFloat(),
                statueJson["rotation"][1].asFloat(),
                statueJson["rotation"][2].asFloat());
            statueRef->setScale(
                Eend::Scale(
                    statueJson["scale"][0].asFloat(),
                    statueJson["scale"][1].asFloat(),
                    statueJson["scale"][2].asFloat()));
        }
    }
    if (rootJson["Dolls"].isArray()) {
        for (unsigned int dollIdx = 0; dollIdx < rootJson["Dolls"].size(); ++dollIdx) {
            Json::Value dollJson = rootJson["Dolls"][dollIdx];
            Eend::DollId id = Eend::Entities::dolls().insert(dollJson["path"].asString());
            Eend::Doll* dollRef = Eend::Entities::dolls().getRef(id);
            float pace = dollJson["pace"].asFloat();
            m_dolls.push_back(std::tie(id, pace));

            float tileXIdx = dollJson["position"][0].asFloat();
            float tileYIdx = dollJson["position"][1].asFloat();
            float heightOffset = dollJson["position"][2].asFloat();

            dollRef->setPosition(positionAtTile(tileXIdx, tileYIdx, heightOffset));
            dollRef->setRotation(
                dollJson["rotation"][0].asFloat(),
                dollJson["rotation"][1].asFloat(),
                dollJson["rotation"][2].asFloat());
            dollRef->setScale(
                Eend::Scale(
                    dollJson["scale"][0].asFloat(),
                    dollJson["scale"][1].asFloat(),
                    dollJson["scale"][2].asFloat()));
            if (dollJson.isMember("animation"))
                dollRef->setAnimation(dollJson["animation"].asString());
        }
    }

    if ((int)m_heightMap.size() == 0) {
        Eend::fatalError("height map is empty");
    }
    if ((int)m_heightMap.size() != (m_height + 1)) {
        Eend::fatalError("height map unexpected height");
    }
    for (auto& line : m_heightMap) {
        if ((int)line.size() != (m_width + 1)) {
            Eend::fatalError("height map unexpected line width");
        }
    }
    // make obj
    std::string fileName = pngHeightMap.stem().string();

    std::filesystem::path objPath = "resources" / path / path.filename().string().append(".obj");
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
    for (auto& line : m_heightMap) {
        for (float& height : line) {
            // x and y Pos also UV map?
            objFile << "v " << xPos * m_scale.x << " " << yPos * m_scale.y << " " << height
                    << std::endl; // height << std::endl;
            xPos += 1.0f;
        }
        xPos = 0.0f;
        yPos -= 1.0f;
    }
    // uv mapping
    for (size_t lineIdx = 0; lineIdx < m_heightMap.size(); ++lineIdx) {
        float heightRatio = 1.0f - ((float)lineIdx / (float)(m_heightMap.size() - 1.0f));
        for (unsigned int eleIdx = 0; eleIdx < m_heightMap[lineIdx].size(); ++eleIdx) {
            float widthRatio = (float)eleIdx / (float)(m_heightMap[lineIdx].size() - 1.0f);
            objFile << "vt " << widthRatio << " " << heightRatio << std::endl;
        }
    }

    objFile << "s 0" << std::endl; // I have no idea
    objFile << "usemtl Material" << std::endl;
    // tris by vert index
    for (size_t lineIdx = 0; lineIdx < (m_heightMap.size() - 1); ++lineIdx) {
        for (unsigned int eleIdx = 0; eleIdx < (m_heightMap[lineIdx].size() - 1); ++eleIdx) {
            // I think this is 1 indexed :c
            unsigned int flatIdx = eleIdx + (lineIdx * m_heightMap[lineIdx].size()) + 1;
            unsigned int rightIdx = flatIdx + 1;
            unsigned int downIdx = flatIdx + m_heightMap[lineIdx].size();
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

    std::ofstream mtlFile(path / (path.filename().string() + ".mtl"));
    mtlFile << "#TreySim4" << std::endl;
    mtlFile << "newmtl Material" << std::endl;
    mtlFile << "Ns 0.000000" << std::endl;
    mtlFile << "Ka 1.000000 1.000000 1.000000" << std::endl;
    mtlFile << "Kd 0.800000 0.800000 0.800000" << std::endl;
    mtlFile << "Ks 0.500000 0.500000 0.500000" << std::endl;
    mtlFile << "Ke 0.000000 0.000000 0.000000" << std::endl;
    mtlFile << "Ni 1.000000" << std::endl;
    mtlFile << "d 1.000000" << std::endl;
    mtlFile << "illum 2" << std::endl;
    mtlFile << "map_Kd diffuse.png" << std::endl;

    mtlFile.close();

    m_statueId = Eend::Entities::statues().insert(path);
}

Terrain::~Terrain() {
    Eend::Entities::statues().erase(m_statueId);
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
    for (auto& doll : m_dolls) {
        Eend::Doll* dollRef = Eend::Entities::dolls().getRef(std::get<Eend::DollId>(doll));
        float animScale = dollRef->getAnim();
        animScale += std::get<float>(doll) * Eend::FrameLimiter::get().deltaTime;
        dollRef->setAnim(animScale);
    }
    for (auto& board : m_boards) {
        Eend::Board* boardRef = Eend::Entities::boards().getRef(std::get<Eend::BoardId>(board));
        if (std::get<float>(board) != 0) {
            boardRef->setStripIdx((size_t)(cumulative / std::get<float>(board)));
        }
    }
}

bool Terrain::colliding(const Eend::Point2D point) {
    for (auto& rectangle : m_collisionRectangles) {
        if (pointOnRectangle(point, rectangle)) return true;
    }
    return false;
}

Eend::Point Terrain::positionAtTile(const float tileXIdx, const float tileYIdx) {
    return positionAtTile(tileXIdx, tileYIdx, 0.0f);
}

Eend::Point
Terrain::positionAtTile(const float tileXIdx, const float tileYIdx, const float heightOffset) {
    return Eend::Point(
        (tileXIdx * m_scale.x) + (m_scale.x / 2.0),
        (tileYIdx * -m_scale.y) + (-m_scale.y / 2.0),
        heightOffset +
            heightAtPoint(Eend::Point2D((float)tileXIdx * m_scale.x, tileYIdx * -m_scale.y)));
}

float Terrain::heightAtPoint(Eend::Point2D point) {

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

    const float scaledX = point.x / m_scale.x;
    const float scaledY = point.y / m_scale.y;

    const bool outsideTerrain = scaledX < 0 || scaledY > 0 ||
                                scaledX >= ((float)m_heightMap[0].size() - 1.0f) ||
                                scaledY <= (-(float)m_heightMap.size() + 1.0f);
    // could make this an optional
    if (outsideTerrain) {
        return 0.0f;
    }

    Eend::Triangle triangle(Eend::Point(0.0f), Eend::Point(0.0f), Eend::Point(0.0f));

    const float relativeX = scaledX - (float)floor(scaledX);
    const float relativeY = -scaledY - (float)floor(-scaledY);
    const bool upperTri = (relativeX - relativeY) > 0;

    const size_t topLeftXIdx = (size_t)floor(scaledX);
    const size_t topLeftYIdx = (size_t)floor(-scaledY);

    const size_t bottomRightXIdx = (size_t)floor(scaledX + 1);
    const size_t bottomRightYIdx = (size_t)floor(-scaledY + 1);

    const Eend::Point topLeftPoint = Eend::Point(
        (float)topLeftXIdx * m_scale.x,
        (float)topLeftYIdx * -m_scale.y,
        m_heightMap[topLeftYIdx][topLeftXIdx]);

    const Eend::Point bottomRightPoint = Eend::Point(
        (float)bottomRightXIdx * m_scale.x,
        (float)bottomRightYIdx * -m_scale.y,
        m_heightMap[bottomRightYIdx][bottomRightXIdx]);

    if (upperTri) {

        const size_t topRightXIdx = (size_t)floor(scaledX + 1);
        const size_t topRightYIdx = (size_t)floor(-scaledY);

        const Eend::Point topRightPoint = Eend::Point(
            (float)topRightXIdx * m_scale.x,
            (float)topRightYIdx * -m_scale.y,
            m_heightMap[topRightYIdx][topRightXIdx]);

        triangle = Eend::Triangle(topLeftPoint, topRightPoint, bottomRightPoint);

    } else {

        const size_t bottomLeftXIdx = (size_t)floor(scaledX);
        const size_t bottomLeftYIdx = (size_t)floor(-scaledY + 1);
        const Eend::Point bottomLeftPoint = Eend::Point(
            (float)bottomLeftXIdx * m_scale.x,
            (float)bottomLeftYIdx * -m_scale.y,
            m_heightMap[bottomLeftYIdx][bottomLeftXIdx]);

        triangle = Eend::Triangle(topLeftPoint, bottomLeftPoint, bottomRightPoint);
    }
    return pointHeightOnTri(triangle, point);
}
