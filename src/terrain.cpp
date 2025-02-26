#include "terrain.hpp"

#include <Eendgine/collisionGeometry.hpp>
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
#include <stdio.h>
#include <string>

namespace Eend = Eendgine;

float pointHeightOnTri(
    const Eend::Point& p1, const Eend::Point& p2, const Eend::Point& p3, float x, float z);
float pythagorean(float a, float b);

Terrain::Terrain(const std::filesystem::path path, Eend::Scale scale)
    : _height(0), _width(0), _statueId(0), _scale(scale) {
    // height map from image
    std::filesystem::path pngHeightMap = "resources" / path / "heightMap.png";
    std::filesystem::path pngCollisionMap = "resources" / path / "collisionMap.png";
    int collisionHeight = 0;
    int collisionWidth = 0;
    int channels = 0;

    unsigned char* imageData = stbi_load(pngHeightMap.c_str(), &_width, &_height, &channels, 0);
    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + pngHeightMap.generic_string() + " for terrain generation");
    }

    for (int h = 0; h < (_height - 1); ++h) {
        // handling edge cases looks horrible ugly, i'll admit
        _heightMap.emplace_back();
        if (h == 0)
            _heightMap.emplace_back();
        if (h == (_height - 2))
            _heightMap.emplace_back();
        for (int w = 0; w < (_width - 1); ++w) {
            unsigned int avg = 0;
            const size_t currentIdx = w + (h * _width);
            const size_t rightIdx = (w + 1) + (h * _width);
            const size_t downIdx = w + ((h + 1) * _width);
            const size_t rightDownIdx = downIdx + 1;
            if (h == 0) {
                if (w == 0) {
                    avg = imageData[currentIdx];
                    _heightMap[h].push_back(((float)avg / 256.0f) * _scale.y);
                }
                avg = (imageData[currentIdx] + imageData[rightIdx]) / 2;
                _heightMap[h].push_back(((float)avg / 256.0f) * _scale.y);
                if (w == (_width - 2)) {
                    avg = imageData[currentIdx];
                    _heightMap[h].push_back(((float)avg / 256.0f) * _scale.y);
                }
            }
            if (h == (_height - 2)) {
                if (w == 0) {
                    avg = imageData[downIdx];
                    _heightMap[h + 2].push_back(((float)avg / 256.0f) * _scale.y);
                }
                avg = (imageData[downIdx] + imageData[rightDownIdx]) / 2;
                _heightMap[h + 2].push_back(((float)avg / 256.0f) * _scale.y);
                if (w == (_width - 2)) {
                    avg = imageData[downIdx];
                    _heightMap[h + 2].push_back(((float)avg / 256.0f) * _scale.y);
                }
            }
            if (w == 0) {
                avg = (imageData[currentIdx] + imageData[downIdx]) / 2;
                _heightMap[h + 1].push_back(((float)avg / 256.0f) * _scale.y);
            }
            avg = ((imageData[currentIdx] + imageData[rightIdx] + imageData[downIdx] +
                       imageData[rightDownIdx]) /
                   4);
            _heightMap[h + 1].push_back(((float)avg / 256.0f) * _scale.y);
            if (w == (_width - 2)) {
                avg = (imageData[currentIdx] + imageData[downIdx]) / 2;
                _heightMap[h + 1].push_back(((float)avg / 256.0f) * _scale.y);
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
                const Eend::Point2D upperLeft((w * _scale.x) + 1, (h * _scale.z) + 1);
                const Eend::Point2D lowerRight(((w + 1) * _scale.x) + 1, ((h + 1) * _scale.z) + 1);
                _collisionRectangles.emplace_back(upperLeft, lowerRight);
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

    if (rootJson["Boards"].isArray()) {
        for (unsigned int boardIdx = 0; boardIdx < rootJson["Boards"].size(); ++boardIdx) {
            Json::Value boardJson = rootJson["Boards"][boardIdx];
            Eend::BoardId id = Eend::Entities::BoardBatch::insert(boardJson["path"].asString());
            Eend::Board* boardRef = Eend::Entities::BoardBatch::getRef(id);
            _boards.push_back(id);

            Eend::Point position(boardJson["position"][0].asFloat(),
                boardJson["position"][1].asFloat(), boardJson["position"][2].asFloat());

            boardRef->setPosition(Eend::Point((position.x * _scale.x) + (_scale.x / 2.0),
                position.y + heightAtPoint(position.x * _scale.x, position.z * _scale.z),
                (position.z * _scale.z) + (_scale.z / 2.0)));
            boardRef->setRotation(boardJson["rotation"].asFloat());
            boardRef->setScale(
                Eend::Scale2D(boardJson["scale"][0].asFloat(), boardJson["scale"][1].asFloat()));
        }
    }
    if (rootJson["Statues"].isArray()) {
        for (unsigned int statueIdx = 0; statueIdx < rootJson["Statues"].size(); ++statueIdx) {
            Json::Value statueJson = rootJson["Statues"][statueIdx];
            Eend::StatueId id = Eend::Entities::StatueBatch::insert(statueJson["path"].asString());
            Eend::Statue* statueRef = Eend::Entities::StatueBatch::getRef(id);
            _statues.push_back(id);

            Eend::Point position(statueJson["position"][0].asFloat(),
                statueJson["position"][1].asFloat(), statueJson["position"][2].asFloat());

            statueRef->setPosition(Eend::Point((position.x * _scale.x) + (_scale.x / 2.0),
                position.y + heightAtPoint(position.x * _scale.x, position.z * _scale.z),
                (position.z * _scale.z) + (_scale.z / 2.0)));
            statueRef->setRotation(
                statueJson["rotation"][0].asFloat(), statueJson["rotation"][1].asFloat());
            statueRef->setScale(Eend::Scale(statueJson["scale"][0].asFloat(),
                statueJson["scale"][1].asFloat(), statueJson["scale"][2].asFloat()));
        }
    }
    if (rootJson["Dolls"].isArray()) {
        for (unsigned int dollIdx = 0; dollIdx < rootJson["Dolls"].size(); ++dollIdx) {
            Json::Value dollJson = rootJson["Dolls"][dollIdx];
            Eend::DollId id = Eend::Entities::DollBatch::insert(dollJson["path"].asString());
            Eend::Doll* dollRef = Eend::Entities::DollBatch::getRef(id);
            float pace = dollJson["pace"].asFloat();
            _dolls.push_back(std::tie(id, pace));

            Eend::Point position(dollJson["position"][0].asFloat(),
                dollJson["position"][1].asFloat(), dollJson["position"][2].asFloat());

            dollRef->setPosition(Eend::Point((position.x * _scale.x) + (_scale.x / 2.0),
                position.y + heightAtPoint(position.x * _scale.x, position.z * _scale.z),
                (position.z * _scale.z) + (_scale.z / 2.0)));
            dollRef->setRotation(
                dollJson["rotation"][0].asFloat(), dollJson["rotation"][1].asFloat());
            dollRef->setScale(Eend::Scale(dollJson["scale"][0].asFloat(),
                dollJson["scale"][1].asFloat(), dollJson["scale"][2].asFloat()));
            if (dollJson.isMember("animation"))
                dollRef->setAnimation(dollJson["animation"].asString());
        }
    }

    if ((int)_heightMap.size() == 0) {
        Eend::fatalError("height map is empty");
    }
    if ((int)_heightMap.size() != (_height + 1)) {
        Eend::fatalError("height map unexpected height");
    }
    for (auto& line : _heightMap) {
        if ((int)line.size() != (_width + 1)) {
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
    for (auto& line : _heightMap) {
        for (float& height : line) {
            // x and y Pos also UV map?
            objFile << "v " << xPos * _scale.x << " " << height << " " << yPos * _scale.z
                    << std::endl; // height << std::endl;
            xPos += 1.0f;
        }
        xPos = 0.0f;
        yPos += 1.0f;
    }
    // uv mapping
    for (size_t lineIdx = 0; lineIdx < _heightMap.size(); ++lineIdx) {
        float heightRatio = 1.0f - ((float)lineIdx / (float)(_heightMap.size() - 1.0f));
        for (unsigned int eleIdx = 0; eleIdx < _heightMap[lineIdx].size(); ++eleIdx) {
            float widthRatio = (float)eleIdx / (float)(_heightMap[lineIdx].size() - 1.0f);
            objFile << "vt " << widthRatio << " " << heightRatio << std::endl;
        }
    }

    objFile << "s 0" << std::endl; // I have no idea
    objFile << "usemtl Material" << std::endl;
    // tris by vert index
    for (size_t lineIdx = 0; lineIdx < (_heightMap.size() - 1); ++lineIdx) {
        for (unsigned int eleIdx = 0; eleIdx < (_heightMap[lineIdx].size() - 1); ++eleIdx) {
            // I think this is 1 indexed :c
            unsigned int flatIdx = eleIdx + (lineIdx * _heightMap[lineIdx].size()) + 1;
            unsigned int rightIdx = flatIdx + 1;
            unsigned int downIdx = flatIdx + _heightMap[lineIdx].size();
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

    _statueId = Eend::Entities::StatueBatch::insert(path);
}

Terrain::~Terrain() {
    Eend::Entities::StatueBatch::erase(_statueId);
    for (Eend::BoardId& board : _boards)
        Eend::Entities::BoardBatch::erase(board);
    for (Eend::StatueId& statue : _statues)
        Eend::Entities::StatueBatch::erase(statue);
    for (auto& doll : _dolls)
        Eend::Entities::DollBatch::erase(std::get<Eend::DollId>(doll));
}

void Terrain::update() {
    for (auto& doll : _dolls) {
        Eend::Doll* dollRef = Eend::Entities::DollBatch::getRef(std::get<Eend::DollId>(doll));
        float animScale = dollRef->getAnim();
        animScale += std::get<float>(doll) * Eend::FrameLimiter::deltaTime;
        dollRef->setAnim(animScale);
    }
}

bool Terrain::colliding(const Eend::Point2D point) {
    for (auto& rectangle : _collisionRectangles) {
        if (Eend::colliding(point, rectangle))
            return true;
    }
    return false;
}

//    top left +-------+ top right
//             |      /|
//             |upper/ |
//             |    /  |
//             |   /   |
//             |  /    |
//             | /lower|
//             |/      |
// bottom left +-------+ bottom right

float Terrain::heightAtPoint(const float x, const float z) {

    const float scaledX = x / _scale.x;
    const float scaledZ = z / _scale.z;
    // IDK WHERE
    // BUT I AM HANDLING THE SCALING WRONG

    if (x < 0 || z < 0 || scaledX >= (_heightMap[0].size() - 1) ||
        scaledZ >= (_heightMap.size() - 1)) {
        // outside of the terrain area
        return 0.0f;
    }

    const float relativeX = scaledX - floor(scaledX);
    const float relativeZ = scaledZ - floor(scaledZ);

    const float topLeftX = floor(scaledX) * _scale.x;
    const float topRightX = (floor(scaledX) + 1) * _scale.x;
    const float bottomRightX = (floor(scaledX) + 1) * _scale.x;
    const float bottomLeftX = floor(scaledX) * _scale.x;

    const float topLeftY = _heightMap[(size_t)floor(scaledZ) + 1][(size_t)floor(scaledX)];
    const float topRightY = _heightMap[(size_t)floor(scaledZ) + 1][(size_t)floor(scaledX) + 1];
    const float bottomRightY = _heightMap[(size_t)floor(scaledZ)][(size_t)floor(scaledX) + 1];
    const float bottomLeftY = _heightMap[(size_t)floor(scaledZ)][(size_t)floor(scaledX)];

    const float topLeftZ = (floor(scaledZ) + 1) * _scale.z;
    const float topRightZ = (floor(scaledZ) + 1) * _scale.z;
    const float bottomRightZ = floor(scaledZ) * _scale.z;
    const float bottomLeftZ = floor(scaledZ) * _scale.z;

    const Eend::Point topLeftPoint = Eend::Point(topLeftX, topLeftY, topLeftZ);
    const Eend::Point topRightPoint = Eend::Point(topRightX, topRightY, topRightZ);
    const Eend::Point bottomRightPoint = Eend::Point(bottomRightX, bottomRightY, bottomRightZ);
    const Eend::Point bottomLeftPoint = Eend::Point(bottomLeftX, bottomLeftY, bottomLeftZ);

    if (relativeZ < relativeX) {
        // lower
        return pointHeightOnTri(topRightPoint, bottomLeftPoint, bottomRightPoint, x, z);
    } else {
        // upper
        return pointHeightOnTri(topLeftPoint, topRightPoint, bottomLeftPoint, x, z);
    }
}

float pythagorean(const float a, const float b) {
    return std::sqrt(std::pow(a, 2.0f) + std::pow(b, 2.0f));
}

float pointHeightOnTri(
    const Eend::Point& p1, const Eend::Point& p2, const Eend::Point& p3, float x, float z) {
    // undefined behavior if plane is parallel
    // WHAT?
    // https://math.stackexchange.com/questions/1154340/how-to-find-the-height-of-a-2d-coordinate-on-a-3d-triangle
    float a = -(p3.z * p2.y - p1.z * p2.y - p3.z * p1.y + p1.y * p2.z + p3.y * p1.z - p2.z * p3.y);
    float b = (p1.z * p3.x + p2.z * p1.x + p3.z * p2.x - p2.z * p3.x - p1.z * p2.x - p3.z * p1.x);
    float c = (p2.y * p3.x + p1.y * p2.x + p3.y * p1.x - p1.y * p3.x - p2.y * p1.x - p2.x * p3.y);
    float d = -a * p1.x - b * p1.y - c * p1.z;
    return -(a * x + c * z + d) / b;
}
