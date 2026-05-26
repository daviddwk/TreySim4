#include <Eendgine/audio.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>

#include <optional>
#include <print>

#include "duck.hpp"
// have static last direction in update function
// use this to get new diruction if there is one, and know that keys are actually being pressed
//
//

Duck::Duck()
    : m_bodyId(
          Eend::Entities::statues().insert(std::filesystem::path("resources/duck/statues/body"))),
      m_headId(
          Eend::Entities::boards().insert(std::filesystem::path("resources/duck/boards/head"))),
      m_position(Eend::Point(0.0f)), m_rotation(Eend::Angle(0.0f)) {

    Eend::Board* head = Eend::Entities::boards().getRef(m_headId);
    head->setScale(Eend::Scale2D(3.5f, 3.5f));
    head->setStrip("eyesOpen");
}

Duck::~Duck() {
    Eend::Entities::statues().erase(m_bodyId);
    Eend::Entities::boards().erase(m_headId);
}

void Duck::construct() {
    assert(m_instance == nullptr);
    m_instance = new Duck();
}

void Duck::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

Duck& Duck::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

void Duck::setPosition(Eend::Point position) { m_position = position; }

Eend::Point Duck::getPosition() { return m_position; };
Eend::Point2D Duck::getPosition2D() { return Eend::Point2D(m_position.x, m_position.y); };
float Duck::getRadius() { return M_DUCK_RADIUS; }

void Duck::update() {

    float dt = Eend::FrameLimiter::get().deltaTime;
    m_rotation = m_rotation + Eend::Angle(100 * dt);

    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    Eend::Board* headRef = Eend::Entities::boards().getRef(m_headId);

    bodyRef->setPosition(Eend::Point(m_position.x - 0.5f, m_position.y, m_position.z + 0.08f));
    headRef->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 3.00f));
    bodyRef->setRotation(Eend::Rotation(0.0f, 0.0f, m_rotation.getDegrees() + 180.0f));
}
