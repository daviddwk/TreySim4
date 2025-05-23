#include "Eendgine/board.hpp"
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
#include <string>

namespace Eend = Eendgine;

float pointHeightOnTri(const Eend::Point& p1, const Eend::Point& p2, const Eend::Point& p3,
    const Eend::Point2D& point);

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
                    _heightMap[h].push_back(((float)avg / 256.0f) * _scale.z);
                }
                avg = (imageData[currentIdx] + imageData[rightIdx]) / 2;
                _heightMap[h].push_back(((float)avg / 256.0f) * _scale.z);
                if (w == (_width - 2)) {
                    avg = imageData[currentIdx];
                    _heightMap[h].push_back(((float)avg / 256.0f) * _scale.z);
                }
            }
            if (h == (_height - 2)) {
                if (w == 0) {
                    avg = imageData[downIdx];
                    _heightMap[h + 2].push_back(((float)avg / 256.0f) * _scale.z);
                }
                avg = (imageData[downIdx] + imageData[rightDownIdx]) / 2;
                _heightMap[h + 2].push_back(((float)avg / 256.0f) * _scale.z);
                if (w == (_width - 2)) {
                    avg = imageData[downIdx];
                    _heightMap[h + 2].push_back(((float)avg / 256.0f) * _scale.z);
                }
            }
            if (w == 0) {
                avg = (imageData[currentIdx] + imageData[downIdx]) / 2;
                _heightMap[h + 1].push_back(((float)avg / 256.0f) * _scale.z);
            }
            avg = ((imageData[currentIdx] + imageData[rightIdx] + imageData[downIdx] +
                       imageData[rightDownIdx]) /
                   4);
            _heightMap[h + 1].push_back(((float)avg / 256.0f) * _scale.z);
            if (w == (_width - 2)) {
                avg = (imageData[currentIdx] + imageData[downIdx]) / 2;
                _heightMap[h + 1].push_back(((float)avg / 256.0f) * _scale.z);
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
                const Eend::Point2D upperLeft((w * _scale.x) + 1, -(((h + 1) * _scale.y) + 1));
                const Eend::Point2D lowerRight(((w + 1) * _scale.x) + 1, -((h * _scale.y) + 1));
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
            float pace = boardJson["pace"].asFloat(); // should be 0 if not there?
            _boards.push_back(std::tie(id, pace));

            float tileXIdx = boardJson["position"][0].asFloat();
            float tileYIdx = boardJson["position"][2].asFloat();
            float heightOffset = boardJson["position"][1].asFloat();

            boardRef->setPosition(positionAtTile(tileXIdx, tileYIdx, heightOffset));
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

            float tileXIdx = statueJson["position"][0].asFloat();
            float tileYIdx = statueJson["position"][2].asFloat();
            float heightOffset = statueJson["position"][1].asFloat();

            statueRef->setPosition(positionAtTile(tileXIdx, tileYIdx, heightOffset));
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

            float tileXIdx = dollJson["position"][0].asFloat();
            float tileYIdx = dollJson["position"][2].asFloat();
            float heightOffset = dollJson["position"][1].asFloat();

            dollRef->setPosition(positionAtTile(tileXIdx, tileYIdx, heightOffset));
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
            objFile << "v " << xPos * _scale.x << " " << yPos * _scale.y << " " << height
                    << std::endl; // height << std::endl;
            xPos += 1.0f;
        }
        xPos = 0.0f;
        yPos -= 1.0f;
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
    for (auto& board : _boards)
        Eend::Entities::BoardBatch::erase(std::get<Eend::BoardId>(board));
    for (Eend::StatueId& statue : _statues)
        Eend::Entities::StatueBatch::erase(statue);
    for (auto& doll : _dolls)
        Eend::Entities::DollBatch::erase(std::get<Eend::DollId>(doll));
}

void Terrain::update() {
    static float cumulative = 0.0f;
    cumulative += Eend::FrameLimiter::deltaTime;
    for (auto& doll : _dolls) {
        Eend::Doll* dollRef = Eend::Entities::DollBatch::getRef(std::get<Eend::DollId>(doll));
        float animScale = dollRef->getAnim();
        animScale += std::get<float>(doll) * Eend::FrameLimiter::deltaTime;
        dollRef->setAnim(animScale);
    }
    for (auto& board : _boards) {
        Eend::Board* boardRef = Eend::Entities::BoardBatch::getRef(std::get<Eend::BoardId>(board));
        if (std::get<float>(board) != 0)
            boardRef->setTextureIdx((size_t)(cumulative / std::get<float>(board)));
    }
}

bool Terrain::colliding(const Eend::Point2D point) {
    for (auto& rectangle : _collisionRectangles) {
        if (Eend::colliding(point, rectangle))
            return true;
    }
    return false;
}

Eend::Point Terrain::positionAtTile(
    const float tileXIdx, const float tileYIdx, const float heightOffset) {
    return Eend::Point((tileXIdx * _scale.x) + (_scale.x / 2.0),
        (tileYIdx * -_scale.y) + (-_scale.y / 2.0),
        heightOffset +
            heightAtPoint(Eend::Point2D((float)tileXIdx * _scale.x, tileYIdx * -_scale.y)));
}

float Terrain::heightAtPoint(Eend::Point2D point) {

    //    top left +-------+ top right
    //             |\      |
    //             | \upper|
    //             |  \    |
    //             |   \   |
    //             |    \  |
    //             |lower\ |
    //             |      \|
    // bottom left +-------+ bottom right

    const float scaledX = point.x / _scale.x;
    const float scaledY = point.y / _scale.y;

    const bool outsideTerrain = scaledX < 0 || scaledY > 0 ||
                                scaledX > ((float)_heightMap[0].size() - 1.0f) ||
                                scaledY < (-(float)_heightMap.size() - 1.0f);
    // could make this an optional
    if (outsideTerrain) {
        return 0.0f;
    }

    const float relativeX = scaledX - (float)floor(scaledX);
    const float relativeY = -scaledY - (float)floor(-scaledY);
    const bool upperTri = (relativeX - relativeY) > 0;

    if (upperTri) {
        const size_t topLeftXIdx = (size_t)floor(scaledX);
        const size_t topLeftYIdx = (size_t)floor(-scaledY);

        const size_t topRightXIdx = (size_t)floor(scaledX + 1);
        const size_t topRightYIdx = (size_t)floor(-scaledY);

        const size_t bottomRightXIdx = (size_t)floor(scaledX + 1);
        const size_t bottomRightYIdx = (size_t)floor(-scaledY + 1);

        const Eend::Point topLeftPoint = Eend::Point((float)topLeftXIdx * _scale.x,
            (float)topLeftYIdx * _scale.y, _heightMap[topLeftYIdx][topLeftXIdx]);
        const Eend::Point topRightPoint = Eend::Point((float)topRightXIdx * _scale.x,
            (float)topRightYIdx * _scale.y, _heightMap[topRightYIdx][topRightXIdx]);
        const Eend::Point bottomRightPoint = Eend::Point((float)bottomRightXIdx * _scale.x,
            (float)bottomRightYIdx * _scale.y, _heightMap[bottomRightYIdx][bottomRightXIdx]);

        return pointHeightOnTri(topLeftPoint, topRightPoint, bottomRightPoint, point);
    } else {
        // lower tri
        const size_t topLeftXIdx = (size_t)floor(scaledX);
        const size_t topLeftYIdx = (size_t)floor(-scaledY);

        const size_t bottomLeftXIdx = (size_t)floor(scaledX);
        const size_t bottomLeftYIdx = (size_t)floor(-scaledY + 1);

        const size_t bottomRightXIdx = (size_t)floor(scaledX + 1);
        const size_t bottomRightYIdx = (size_t)floor(-scaledY + 1);

        const Eend::Point topLeftPoint = Eend::Point((float)topLeftXIdx * _scale.x,
            (float)topLeftYIdx * _scale.y, _heightMap[topLeftYIdx][topLeftXIdx]);
        const Eend::Point bottomLeftPoint = Eend::Point((float)bottomLeftXIdx * _scale.x,
            (float)bottomLeftYIdx * _scale.y, _heightMap[bottomLeftYIdx][bottomLeftXIdx]);
        const Eend::Point bottomRightPoint = Eend::Point((float)bottomRightXIdx * _scale.x,
            (float)bottomRightYIdx * _scale.y, _heightMap[bottomRightYIdx][bottomRightXIdx]);

        return pointHeightOnTri(topLeftPoint, bottomRightPoint, bottomLeftPoint, point);
    }

    /*
        const float scaledX = point.x / _scale.x;
        const float scaledY = point.y / _scale.y;
        // IDK WHERE
        // BUT I AM HANDLING THE SCALING WRONG

        if (point.x < 0 || point.y > 0 || scaledX >= (_heightMap[0].size() - 1) ||
            scaledY >= (_heightMap.size() - 1)) {
            // outside of the terrain area
            return 0.0f;
        }

        const float relativeX = scaledX - floor(scaledX);
        const float relativeY = scaledY - floor(scaledY);

        const float topLeftX = floor(scaledX) * _scale.x;
        const float topRightX = (floor(scaledX) + 1) * _scale.x;
        const float bottomRightX = (floor(scaledX) + 1) * _scale.x;
        const float bottomLeftX = floor(scaledX) * _scale.x;

        const float topLeftY = (floor(scaledY) + 1) * _scale.y;
        const float topRightY = (floor(scaledY) + 1) * _scale.y;
        const float bottomRightY = floor(scaledY) * _scale.y;
        const float bottomLeftY = floor(scaledY) * _scale.y;


    const float topLeftZ = _heightMap[(size_t)floor(-scaledY) + 1][(size_t)floor(scaledX)];
    const float topRightZ = _heightMap[(size_t)floor(-scaledY) + 1][(size_t)floor(scaledX) + 1];
    const float bottomRightZ = _heightMap[(size_t)floor(-scaledY)][(size_t)floor(scaledX) + 1];
    const float bottomLeftZ = _heightMap[(size_t)floor(-scaledY)][(size_t)floor(scaledX)];

    const Eend::Point topLeftPoint = Eend::Point(topLeftX, topLeftY, topLeftZ);
    const Eend::Point topRightPoint = Eend::Point(topRightX, topRightY, topRightZ);
    const Eend::Point bottomRightPoint = Eend::Point(bottomRightX, bottomRightY, bottomRightZ);
    const Eend::Point bottomLeftPoint = Eend::Point(bottomLeftX, bottomLeftY, bottomLeftZ);

    if (relativeY < relativeX) {
        // lower
        std::print("tr{} {} {}\nbl{} {} {}\nbr{} {} {}\np{} {}\n", topRightPoint.x,
    topRightPoint.y, topRightPoint.z, bottomLeftPoint.x, bottomLeftPoint.y, bottomLeftPoint.z,
            bottomRightPoint.x, bottomRightPoint.y, bottomRightPoint.z, point.x, point.y);
        return pointHeightOnTri(topRightPoint, bottomLeftPoint, bottomRightPoint, point);
    } else {
        // upper
        return pointHeightOnTri(topLeftPoint, topRightPoint, bottomLeftPoint, point);
    }
    */
}

inline float pointHeightOnTri(const Eend::Point& p1, const Eend::Point& p2, const Eend::Point& p3,
    const Eend::Point2D& point) {
    // undefined behavior if plane is parallel
    // WHAT?
    // https://math.stackexchange.com/questions/1154340/how-to-find-the-height-of-a-2d-coordinate-on-a-3d-triangle
    // float a = -(p3.Z * p2.Y - p1.Z * p2.Y - p3.Z * p1.Y + p1.Y * p2.Z + p3.Y * p1.Z - p2.Z *
    // p3.Y); float b = (p1.Z * p3.X + p2.Z * p1.X + p3.Z * p2.X - p2.Z * p3.X - p1.Z * p2.X - p3.Z
    // * p1.X); float c = (p2.Y * p3.X + p1.Y * p2.X + p3.Y * p1.X - p1.Y * p3.X - p2.Y * p1.X -
    // p2.X * p3.Y); float d = -a * p1.X - b * p1.Y - c * p1.Z; return -(a * x + b * y + d) / c;
    return p1.z;
}
