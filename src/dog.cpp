#include "Eendgine/types.hpp"
#include "dog.hpp"

#include <glm/vector_relational.hpp>
#include <print>
namespace Eend = Eendgine;

Dog::Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Terrain* terrain)
    : m_bodyId(Eend::Entities::boards().insert(std::filesystem::path("dog/boards"))),
      m_position(position), m_speed(speed), m_knockback(Eend::Point2D(0.0f)), m_terrain(terrain),
      m_time(0.0f), m_health(1), m_deadTime(0.0f), m_delete(false) {
    Eend::Board* boardRef = Eend::Entities::boards().getRef(m_bodyId);
    boardRef->setStrip("walk");
    boardRef->setScale(scale);
    boardRef->setPosition(Eend::Point(position.x, terrain->heightAtPoint(position), position.y));
}

void Dog::eraseEntities() { Eend::Entities::boards().erase(m_bodyId); }

void Dog::setSpeed(float speed) { m_speed = speed; }

Eend::Point2D Dog::getPosition() { return m_position; }

Eend::Point Dog::getPosition3d() {
    return Eend::Point(m_position.x, m_position.y, m_terrain->heightAtPoint(m_position));
}

unsigned int Dog::getDamage() { return M_DAMAGE; }

void Dog::giveDamage(unsigned int damage) {
    if (damage >= m_health) {
        m_health = 0;
        Eend::Entities::boards().getRef(m_bodyId)->setStrip("dead");
    } else {
        m_health -= damage;
    }
}

unsigned int Dog::getHealth() { return m_health; }

bool Dog::shouldDelete() { return m_delete; }

void Dog::kick(Eend::Point kick) {
    // make kick power scale exponentially
    float kickFactor = glm::pow(glm::length(kick), 2.0f);
    kickFactor = ((M_KNOCKBACK_MAX - M_KNOCKBACK_MIN) * kickFactor) + (M_KNOCKBACK_MIN);
    m_knockback = glm::normalize(kick) * kickFactor;
}

void Dog::update(float dt, Eend::Point2D approachPoint) {
    m_time += dt;
    if (m_health == 0) {
        m_deadTime += dt;
    }
    Eend::Board* boardRef = Eend::Entities::boards().getRef(m_bodyId);

    m_position += m_knockback * dt;

    if (m_health != 0) {
        const glm::vec2 difference = approachPoint - m_position;
        boardRef->setFlip(difference.x < 0.0f);
        if (glm::length(difference) > M_STOP_DISTANCE) {
            m_position += glm::normalize(approachPoint - m_position) * M_SPEED * dt;
        }
    }

    m_knockback = m_knockback / (1 + (dt * M_KNOCKBACK_DECAY_FACTOR));

    if (m_health == 0 && (glm::length(m_knockback) < M_KNOCKBACK_STOP)) {
        m_knockback = Eend::Vector2D(0.0f);
    }

    if (m_deadTime > M_DELETE_AFTER_DEATH_TIME) {
        m_delete = true;
    }

    // add some offsets for the dog visually here
    boardRef->setPosition(
        Eend::Point(
            m_position.x, m_position.y, m_terrain->heightAtPoint(m_position) + M_UP_OFFSET));
    if (m_time > M_ANIM_INCREMENT_TIME) {
        m_time = 0;
        boardRef->nextStripIdx();
    }
}
