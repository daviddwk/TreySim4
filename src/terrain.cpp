#include "terrain.hpp"

#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/fatalError.hpp>
#include <Eendgine/info.hpp>
#include <Eendgine/types.hpp>

#include <stb/stb_image.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

namespace Eend = Eendgine;

float pointHeightOnTri(
    const Eend::Point& p1, const Eend::Point& p2, const Eend::Point& p3, float x, float z);
float pythagorean(float a, float b);

Terrain::Terrain(const std::filesystem::path path, Eend::Scale scale)
    : _height(0), _width(0), _statueId(0), _scale(scale) {

    // this should be checked by Statue initializer

    // if (!std::filesystem::is_directory(path)) {
    //     Eend::fatalError("loading terrain " + path.string() + " is not a directory");
    // }

    std::filesystem::path pngHeightMap = "resources" / path / (path.filename().string() + ".png");

    int channels = 0;
    unsigned char* imageData = stbi_load(pngHeightMap.c_str(), &_width, &_height, &channels, 0);
    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + pngHeightMap.generic_string() + " for terrain generation");
    }

    for (int h = 0; h < (_height - 1); ++h) {
        _heightMap.emplace_back();
        for (int w = 0; w < (_width - 1); ++w) {
            const unsigned int currentIdx = w + (h * _width);
            const unsigned int rightIdx = (w + 1) + (h * _width);
            const unsigned int downIdx = w + ((h + 1) * _width);
            const unsigned int avg = ((imageData[currentIdx] + imageData[rightIdx] +
                                          imageData[downIdx] + imageData[currentIdx]) /
                                      4);
            _heightMap[h].push_back(((float)avg / 256.0f) * _scale.y);
        }
    }

    stbi_image_free(imageData);

    if ((int)_heightMap.size() == 0) {
        Eend::fatalError("height map is empty");
    }
    if ((int)_heightMap.size() != (_height - 1)) {
        Eend::fatalError("height map unexpected height");
    }
    for (auto& line : _heightMap) {
        if ((int)line.size() != (_width - 1)) {
            Eend::fatalError("height map unexpected line width");
        }
    }
    // make obj
    std::string fileName = pngHeightMap.stem().string();

    std::ofstream objFile(path / (path.filename().string() + ".obj"));

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
    for (unsigned int lineIdx = 0; lineIdx < _heightMap.size(); ++lineIdx) {
        float heightRatio = 1.0f - ((float)lineIdx / (float)(_heightMap.size() - 1.0f));
        for (unsigned int eleIdx = 0; eleIdx < _heightMap[lineIdx].size(); ++eleIdx) {
            float widthRatio = (float)eleIdx / (float)(_heightMap[lineIdx].size() - 1.0f);
            objFile << "vt " << widthRatio << " " << heightRatio << std::endl;
        }
    }

    objFile << "s 0" << std::endl; // I have no idea
    objFile << "usemtl Material" << std::endl;
    // tris by vert index
    for (unsigned int lineIdx = 0; lineIdx < (_heightMap.size() - 1); ++lineIdx) {

        for (unsigned int eleIdx = 0; eleIdx < (_heightMap[lineIdx].size() - 1); ++eleIdx) {

            // I think this is 1 indexed :c
            unsigned int flatIdx = eleIdx + (lineIdx * _heightMap[lineIdx].size()) + 1;
            unsigned int rightIdx = flatIdx + 1;
            unsigned int downIdx = flatIdx + _heightMap[lineIdx].size();
            unsigned int rightDownIdx = downIdx + 1;

            objFile << "f " << flatIdx << '/' << flatIdx << '/' << flatIdx << ' ';
            objFile << rightIdx << '/' << rightIdx << '/' << rightIdx << ' ';
            objFile << rightDownIdx << '/' << rightDownIdx << '/' << rightDownIdx << std::endl;

            objFile << "f " << flatIdx << '/' << flatIdx << '/' << flatIdx << ' ';
            objFile << downIdx << '/' << downIdx << '/' << downIdx << ' ';
            objFile << rightDownIdx << '/' << rightDownIdx << '/' << rightDownIdx << std::endl;
        }
    }

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

    objFile.close();
    mtlFile.close();

    _statueId = Eend::Entities::StatueBatch::insert(path);
}

Terrain::~Terrain() { Eend::Entities::StatueBatch::erase(_statueId); }

//    top left +-------+ top right
//             |\      |
//             | \upper|
//             |  \    |
//             |   \   |
//             |    \  |
//             |lower\ |
//             |      \|
// bottom left +-------+ bottom right

float Terrain::heightAtPoint(const float x, const float z) {

    Eend::Info::registerInt("heightAtPoint", 0);
    Eend::Info::registerFloat("x", 0);
    Eend::Info::registerFloat("z", 0);
    Eend::Info::updateFloat("x", x);
    Eend::Info::updateFloat("z", z);
    Eend::Info::registerInt("hm[z]", 0);
    Eend::Info::registerInt("hm[x]", 0);
    Eend::Info::registerInt("hm[0].size()", 0);
    Eend::Info::registerInt("hm.size()", 0);
    Eend::Info::updateInt("hm[0].size()", _heightMap[0].size());
    Eend::Info::updateInt("hm.size()", _heightMap.size());

    const float scaledX = x / _scale.x;
    const float scaledZ = z / _scale.z;
    Eend::Info::registerFloat("scaledX", 0);
    Eend::Info::registerFloat("scaledZ", 0);
    Eend::Info::updateFloat("scaledX", scaledX);
    Eend::Info::updateFloat("scaledZ", scaledZ);
    Eend::Info::print();
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

    const float topLeftY = _heightMap[(int)floor(scaledZ) + 1][(int)floor(scaledX)];
    const float topRightY = _heightMap[(int)floor(scaledZ) + 1][(int)floor(scaledX) + 1];
    const float bottomRightY = _heightMap[(int)floor(scaledZ)][(int)floor(scaledX) + 1];
    const float bottomLeftY = _heightMap[(int)floor(scaledZ)][(int)floor(scaledX)];

    const float topLeftZ = (floor(scaledZ) + 1) * _scale.z;
    const float topRightZ = (floor(scaledZ) + 1) * _scale.z;
    const float bottomRightZ = floor(scaledZ) * _scale.z;
    const float bottomLeftZ = floor(scaledZ) * _scale.z;

    const Eend::Point topLeftPoint = Eend::Point(topLeftX, topLeftY, topLeftZ);
    const Eend::Point topRightPoint = Eend::Point(topRightX, topRightY, topRightZ);
    const Eend::Point bottomRightPoint = Eend::Point(bottomRightX, bottomRightY, bottomRightZ);
    const Eend::Point bottomLeftPoint = Eend::Point(bottomLeftX, bottomLeftY, bottomLeftZ);

    if (relativeZ < relativeX) {
        // lower tri
        // return topLeftHeight;
        return pointHeightOnTri(topLeftPoint, topRightPoint, bottomRightPoint, x, z);
    } else {
        // upper tri
        // return topLeftHeight;
        return pointHeightOnTri(topLeftPoint, bottomLeftPoint, bottomRightPoint, x, z);
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
