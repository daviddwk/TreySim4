#include "park.hpp"
#include "puppyMill.hpp"
#include "trey.hpp"

// should be path to park format, not png height map
Park::Park(std::filesystem::path pngHeightMap)
    : m_terrain(new Terrain(pngHeightMap)), m_puppyMill(std::make_unique<PuppyMill>(m_terrain)) {}

void Park::construct(std::filesystem::path terrainPath) {
    assert(m_instance == nullptr);
    m_instance = new Park(terrainPath);
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

void Park::update() {

    m_puppyMill->update();
    m_terrain->update();

    if (m_nextTerrainPath) {
        m_terrain = new Terrain(*m_nextTerrainPath);
        m_puppyMill->setTerrain(m_terrain);
        // after because then the new spawn is set
        reset();
    }
    m_nextTerrainPath = std::nullopt;
}

void Park::reset() {
    m_puppyMill.reset(new PuppyMill(m_terrain));
    Trey::get().setPosition(m_terrain->getSpawn());
}

unsigned int Park::numDogsKilled() { return m_puppyMill->getNumKilled(); }

void Park::setTerrain(std::filesystem::path terrainPath) { m_nextTerrainPath = terrainPath; }

bool Park::colliding(Eend::Point2D point) { return m_terrain->colliding(point); };

float Park::getHeight() { return m_terrain->getHeight(); }

float Park::getWidth() { return m_terrain->getWidth(); }

Eend::Point Park::getSpawn() { return m_terrain->getSpawn(); }

float Park::heightAtPoint(Eend::Point2D point) { return m_terrain->heightAtPoint(point); }

Eend::Point Park::positionAtTile(Tile tile) { return m_terrain->positionAtTile(tile); }

Eend::Point Park::positionAtTile(Tile tile, float heightOffset) {
    return m_terrain->positionAtTile(tile, heightOffset);
}
