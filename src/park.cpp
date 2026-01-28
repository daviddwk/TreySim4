#include "park.hpp"

// should be path to park format, not png height map
Park::Park(std::filesystem::path pngHeightMap, Eend::Scale scale, Duck* duck)
    : m_terrain(pngHeightMap, scale), m_duck(duck) {}

void Park::construct(std::filesystem::path pngHeightMap, Eend::Scale scale, Duck* duck) {
    assert(m_instance == nullptr);
    m_instance = new Park(pngHeightMap, scale, duck);
}

void Park::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

Park& Park::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

void Park::setTerrain(std::filesystem::path pngHeightMap, Eend::Scale scale) {
    // m_duck->setPosition(/* ?? */);
}

// TODO instead use passthrough functions for all of the terrain stuff
Terrain& Park::getTerrain() { return m_terrain; }
