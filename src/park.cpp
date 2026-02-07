#include "park.hpp"

#include "duck.hpp"

// should be path to park format, not png height map
Park::Park(std::filesystem::path pngHeightMap, Eend::Scale scale)
    : m_terrain(std::make_unique<Terrain>(pngHeightMap, scale)),
      m_puppyMill(std::make_unique<PuppyMill>()) {}

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

void Park::update(float dt) {
    m_puppyMill->update(dt);
    if (m_nextTerrain) {
        reset();
        Duck::get().setPosition(m_terrain->getSpawn());
    }
    m_nextTerrain = std::nullopt;
}

void Park::reset() {
    std::print("park reset \n");
    // TODO also reset duck position to default
    m_terrain.reset(new Terrain(m_nextTerrain->pngHeightMap, m_nextTerrain->scale));
    m_puppyMill.reset(new PuppyMill());
}

unsigned int Park::numDogsKilled() { return m_puppyMill->getNumKilled(); }

void Park::setTerrain(std::filesystem::path pngHeightMap, Eend::Scale scale) {
    m_nextTerrain = NextTerrainParams(pngHeightMap, scale);
}

bool Park::colliding(Eend::Point2D point) { return m_terrain->colliding(point); };

float Park::getHeight() { return m_terrain->getHeight(); }

float Park::getWidth() { return m_terrain->getWidth(); }

Eend::Point Park::getSpawn() { return m_terrain->getSpawn(); }

float Park::heightAtPoint(Eend::Point2D point) { return m_terrain->heightAtPoint(point); }

Eend::Point Park::positionAtTile(Terrain::Tile tile) { return m_terrain->positionAtTile(tile); }

Eend::Point Park::positionAtTile(Terrain::Tile tile, float heightOffset) {
    return m_terrain->positionAtTile(tile, heightOffset);
}
