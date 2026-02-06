#include "Eendgine/board.hpp"
#include "Eendgine/types.hpp"
#include "dog.hpp"
#include "park.hpp"

#include <glm/vector_relational.hpp>
#include <print>
namespace Eend = Eendgine;

Dog::Dog(Eend::Point2D position, Eend::Scale2D scale, float speed)
    : m_bodyId(Eend::Entities::boards().insert(std::filesystem::path("dog/boards"))),
      m_position(position), m_speed(speed), m_knockback(Eend::Point2D(0.0f)), m_animTime(0.0f),
      m_health(M_HEALTH), m_deadTime(0.0f), m_delete(false) {
    Eend::Board* boardRef = Eend::Entities::boards().getRef(*m_bodyId);
    boardRef->setStrip("walk");
    boardRef->setScale(scale);
    boardRef->setPosition(
        Eend::Point(position.x, Park::get().getTerrain()->heightAtPoint(position), position.y));
}
Dog::~Dog() {
    if (m_bodyId) Eend::Entities::boards().erase(*m_bodyId);
}

Dog::Dog(Dog&& other) noexcept
    : m_bodyId(std::move(other.m_bodyId)), m_position(std::move(other.m_position)),
      m_speed(std::move(other.m_speed)), m_knockback(std::move(other.m_knockback)),
      m_animTime(std::move(other.m_animTime)), m_health(std::move(other.m_health)),
      m_deadTime(std::move(other.m_deadTime)), m_delete(std::move(other.m_delete))

{
    other.m_bodyId = std::nullopt;
}

Dog& Dog::operator=(Dog&& other) noexcept {

    // Self-assignment detection
    if (&other == this) return *this;

    // delete entitiy
    if (m_bodyId) Eend::Entities::boards().erase(*m_bodyId);

    // transfer ownership
    m_bodyId = other.m_bodyId;
    m_position = other.m_position;
    m_speed = other.m_speed;
    m_knockback = other.m_knockback;
    m_animTime = other.m_animTime;
    m_health = other.m_health;
    m_deadTime = other.m_deadTime;
    m_delete = other.m_delete;

    // release ownership
    other.m_bodyId = std::nullopt;

    return *this;
}

void Dog::setSpeed(float speed) { m_speed = speed; }

Eend::Point2D Dog::getPosition() { return m_position; }

Eend::Point Dog::getPosition3d() {
    return Eend::Point(
        m_position.x,
        m_position.y,
        Park::get().getTerrain()->heightAtPoint(m_position));
}

unsigned int Dog::getDamage() { return M_DAMAGE; }

bool Dog::giveDamage(unsigned int damage) {
    if (damage >= m_health) {
        m_health = 0;
        assert(m_bodyId);
        Eendgine::Board* board = Eend::Entities::boards().getRef(*m_bodyId);
        board->setStrip("dead");
        board->setStripIdx(0);
        m_knockback *= M_KNOCKBACK_DEAD_MULTIPLIER;
        return true;
    } else {
        m_health -= damage;
        return false;
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

    assert(m_bodyId);
    Eend::Board* boardRef = Eend::Entities::boards().getRef(*m_bodyId);

    m_animTime += dt;
    if (m_health == 0) {
        m_deadTime += dt;
    }
    if (m_health > 0) {
        if (m_animTime > M_ANIM_INCREMENT_TIME) {
            m_animTime = 0;
            boardRef->nextStripIdx();
        }
    } else {
        if ((m_deadTime > M_BLINK_AFTER_DEATH_TIME) && (m_animTime > M_ANIM_INCREMENT_TIME)) {
            m_animTime = 0;
            boardRef->nextStripIdx();
        }
    }

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
            m_position.x,
            m_position.y,
            Park::get().getTerrain()->heightAtPoint(m_position) + M_UP_OFFSET));
}
