#include "terrain.hpp"

#include <Eendgine/fatalError.hpp>

#include <stb/stb_image.h>

#include <fstream>
#include <iostream>
#include <string>

namespace Eend = Eendgine;

Terrain::Terrain() : _height(0), _width(0) {}
Terrain::~Terrain() {}

std::optional<std::filesystem::path> Terrain::generate_terrain(std::filesystem::path pngHeightMap) {

    int channels = 0;
    unsigned char *imageData = stbi_load(pngHeightMap.c_str(), &_width, &_height, &channels, 0);
    if (imageData == NULL) {
        Eend::fatalError(
            "unable to load image " + pngHeightMap.generic_string() + " for terrain generation");
    }

    for (int h = 0; h < (_height - 1); ++h) {
        _heightMap.emplace_back();
        for (int w = 0; w < (_width - 1); ++w) {
            const unsigned int currentIdx = w + (h * w);
            const unsigned int rightIdx = (w + 1) + (h * w);
            const unsigned int downIdx = w + ((h + 1) * w);
            const unsigned int avg = ((imageData[currentIdx] + imageData[rightIdx] +
                                          imageData[downIdx] + imageData[currentIdx]) /
                                      4);
            _heightMap[h].push_back(avg);
        }
    }

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

    std::ofstream objFile("resources/" + fileName + ".obj");

    objFile << "# TreySim4" << std::endl;
    objFile << "mtllib " << fileName << ".mtl" << std::endl;
    objFile << "o " << fileName << std::endl;

    float scale = 1;
    float xPos, yPos = 0;
    // vert positions
    for (auto &line : _heightMap) {
        for (int &height : line) {
            xPos += 1.0f * scale;
            // x and y Pos also UV map?
            objFile << "v " << xPos << " " << yPos << " " << height << std::endl;
        }
        xPos = 0.0f;
        yPos += 1.0f * scale;
    }
    // vert normals
    for (auto &line : _heightMap) {
        for (int &height : line) {
            // temp vertex normals
            objFile << "vn " << 0.000f << " " << 0.000f << " " << 1.000f << std::endl;
        }
    }
    // uv mapping
    for (unsigned int lineIdx = 0; lineIdx < _heightMap.size(); ++lineIdx) {
        float heightRatio = (float)lineIdx / (float)(_heightMap.size() - 1);
        for (unsigned int eleIdx = 0; eleIdx < _heightMap[lineIdx].size(); ++eleIdx) {
            float widthRatio = (float)eleIdx / (float)(_heightMap[lineIdx].size() - 1);
            objFile << "vt " << widthRatio << " " << 0.000f << heightRatio << std::endl;
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

    std::ofstream mtlFile("resources/" + fileName + ".mtl");
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
    mtlFile << "map_Kd " << fileName << ".png" << std::endl;

    return std::nullopt;
}
