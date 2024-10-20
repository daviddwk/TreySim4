#include "terrain.hpp"

#include <Eendgine/fatalError.hpp>

#include <stb/stb_image.h>

#include <fstream>
#include <iostream>
#include <string>

namespace Eend = Eendgine;

Terrain::Terrain(std::filesystem::path path, glm::vec3 scale) : _height(0), _width(0), _modelId(0) {

    if (!std::filesystem::is_directory(path)) {
        Eend::fatalError("loading terrain " + path.string() + " is not a directory");
    }

    std::filesystem::path pngHeightMap = path / (path.filename().string() + ".png");

    int channels = 0;
    unsigned char *imageData = stbi_load(pngHeightMap.c_str(), &_width, &_height, &channels, 0);
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
            _heightMap[h].push_back((float)avg * scale.y);
        }
    }

    stbi_image_free(imageData);

    if (_heightMap.size() == 0) {
        Eend::fatalError("height map is empty");
    }
    if (_heightMap.size() != (_height - 1)) {
        Eend::fatalError("height map unexpected height");
    }
    for (auto &line : _heightMap) {
        if (line.size() != (_width - 1)) {
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
    for (auto &line : _heightMap) {
        for (int &height : line) {
            // x and y Pos also UV map?
            objFile << "v " << xPos * scale.x << " " << height << " " << yPos * scale.z
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

    _modelId = Eend::Entities::ModelBatch::insert(path / (path.filename().string() + ".obj"));
    auto modelRef = Eend::Entities::ModelBatch::getRef(_modelId);
    modelRef.setScale(scale);
}

Terrain::~Terrain() { Eend::Entities::ModelBatch::erase(_modelId); }
