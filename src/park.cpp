#include "park.hpp"
#include "puppyMill.hpp"
#include "trey.hpp"

// should be path to park format, not png height map
Park::Park(std::filesystem::path parkPath)
    : m_terrain(std::make_shared<Terrain>(parkPath)),
      m_puppyMill(std::make_unique<PuppyMill>(m_terrain, parkPath)), m_path(parkPath) {}

void Park::construct(std::filesystem::path parkPath) {
    assert(m_instance == nullptr);
    m_instance = new Park(parkPath);
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

    if (m_nextParkPath) {
        m_terrain.reset(new Terrain(*m_nextParkPath));
        // after because then the new spawn is set
        reset();
    }
    m_nextParkPath = std::nullopt;
}

void Park::reset() {
    m_puppyMill.reset(new PuppyMill(m_terrain, m_path));
    Trey::get().setPosition(m_terrain->getSpawn());
}

bool Park::setWaveIdx(std::vector<Dog>::size_type waveIdx) {
    return m_puppyMill->setWaveIdx(waveIdx);
}

bool Park::nextWave() { return m_puppyMill->nextWave(); }

unsigned int Park::numDogsKilled() { return m_puppyMill->getNumKilled(); }

void Park::setTerrain(std::filesystem::path parkPath) { m_nextParkPath = parkPath; }

bool Park::colliding(Eend::Point2D point) { return m_terrain->colliding(point); };

float Park::getHeight() { return m_terrain->getHeight(); }

float Park::getWidth() { return m_terrain->getWidth(); }

Eend::Point Park::getSpawn() { return m_terrain->getSpawn(); }

float Park::heightAtPoint(Eend::Point2D point) { return m_terrain->heightAtPoint(point); }

Eend::Point Park::positionAtTile(Tile tile) { return m_terrain->positionAtTile(tile); }

Eend::Point Park::positionAtTile(Tile tile, float heightOffset) {
    return m_terrain->positionAtTile(tile, heightOffset);
}
