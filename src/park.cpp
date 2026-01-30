#include "park.hpp"

#include "duck.hpp"

// should be path to park format, not png height map
Park::Park(std::filesystem::path pngHeightMap, Eend::Scale scale)
    : m_terrain(pngHeightMap, scale), m_puppyMill() {}

void Park::construct(std::filesystem::path pngHeightMap, Eend::Scale scale) {
    assert(m_instance == nullptr);
    m_instance = new Park(pngHeightMap, scale);
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

void Park::update(float dt) { m_puppyMill.update(dt, Duck::get()); }
void Park::reset() {
    // TODO also reset duck position to default
    m_puppyMill = PuppyMill();
}

unsigned int Park::numDogsKilled() { return m_puppyMill.getNumKilled(); }

void Park::setTerrain(std::filesystem::path pngHeightMap, Eend::Scale scale) {
    // m_duck->setPosition(/* ?? */);
}

// TODO instead use passthrough functions for all of the terrain stuff
Terrain& Park::getTerrain() { return m_terrain; }
