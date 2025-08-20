#include "dog.hpp"

#include <glm/vector_relational.hpp>
namespace Eend = Eendgine;

const float DOG_UP_OFFSET = 2.0f;
const float DOG_SPEED = 20.0f;
const float DOG_CLOSE_ENOUGH = 1.0f;
const float DOG_ANIM_INCREMENT_TIME = 0.25f;

Dog::Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Terrain* terrain)
    : m_bodyId(Eend::Entities::boards().insert(std::filesystem::path("dog/boards/walk"))),
      m_position(position), m_speed(speed), m_terrain(terrain), m_time(0.0f) {
    Eend::Entities::boards().getRef(m_bodyId)->setScale(scale);
    Eend::Entities::boards().getRef(m_bodyId)->setPosition(
        Eend::Point(position.x, terrain->heightAtPoint(position), position.y));
}

Dog::~Dog() { Eend::Entities::boards().erase(m_bodyId); }

void Dog::setSpeed(float speed) { m_speed = speed; }

Eend::Point2D Dog::getPosition() { return m_position; };

void Dog::update(float dt, Eend::Point2D approachPoint) {
    m_time += dt;
    const glm::vec2 difference = approachPoint - m_position;
    Eend::Entities::boards().getRef(m_bodyId)->setFlip(difference.x < 0.0f);
    if (glm::length(difference) > DOG_CLOSE_ENOUGH) {
        m_position += glm::normalize(approachPoint - m_position) * DOG_SPEED * dt;
    }
    // add some offsets for the dog visually here
    Eend::Entities::boards().getRef(m_bodyId)->setPosition(
        Eend::Point(
            m_position.x, m_position.y, m_terrain->heightAtPoint(m_position) + DOG_UP_OFFSET));
    if (m_time > DOG_ANIM_INCREMENT_TIME) {
        m_time = 0;
        Eend::Entities::boards().getRef(m_bodyId)->nextStripIdx();
    }

    // terrain->heightAtPoint(_position)
}
