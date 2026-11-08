#include "Eendgine/board.hpp"
#include "Eendgine/frameLimiter.hpp"
#include "Eendgine/types.hpp"

#include "dog.hpp"
#include "park.hpp"
#include "trey.hpp"

#include <algorithm>
#include <glm/vector_relational.hpp>
#include <print>
namespace Eend = Eendgine;

constexpr float SPEED = 15.0f;

constexpr unsigned int DAMAGE = 2;
constexpr unsigned int HEALTH = 3;

constexpr float KNOCKBACK_MAX = SPEED * 4.0f;
constexpr float KNOCKBACK_MIN = SPEED;
constexpr float KNOCKBACK_DECAY_FACTOR = 1.0f;
constexpr float KNOCKBACK_STOP = 5.0;
constexpr float KNOCKBACK_DEAD_MULTIPLIER = 1.5f;

constexpr float ANIM_INCREMENT_TIME = 0.25f;
constexpr float QUICK_INCREMENT_TIME = 0.125f;

constexpr float UP_OFFSET = 2.0f;
constexpr float STOP_DISTANCE = 3.0f;

constexpr float BLINK_AFTER_DEATH_TIME = 3.0f;
constexpr float DELETE_AFTER_DEATH_TIME = 5.0f;

Dog::Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Dog::Type type)
    : type(type), body_id(Eend::Entities::boards().insert(getDogPath(type) / "boards")),
      position(position), speed(speed), knockback(Eend::Point2D(0.0f)), anim_time(0.0f),
      health(HEALTH), dead_time(0.0f), to_delete(false) {
    Eend::Board* boardRef = Eend::Entities::boards().getRef(body_id);
    boardRef->setStrip("walk");
    boardRef->setScale(scale);
    boardRef->setPosition(
        Eend::Point(position.x, Park::get().elevationAtPoint(position), position.y));
}
Dog::~Dog() { Eend::Entities::boards().erase(body_id); }

Dog::Dog(const Dog& other) noexcept
    : body_id(Eend::Entities::boards().clone(other.body_id)), position(other.position),
      speed(other.speed), knockback(other.knockback), anim_time(other.anim_time),
      health(other.health), dead_time(other.dead_time), to_delete(other.to_delete) {}

Dog& Dog::operator=(const Dog& other) noexcept {
    // Self-assignment detection
    if (&other == this) return *this;

    Eend::Entities::boards().erase(body_id);

    // transfer ownership
    body_id = Eend::Entities::boards().clone(other.body_id);
    position = other.position;
    speed = other.speed;
    knockback = other.knockback;
    anim_time = other.anim_time;
    health = other.health;
    dead_time = other.dead_time;
    to_delete = other.to_delete;

    return *this;
}

/*
Dog::Dog(Dog&& other) noexcept
    : m_bodyId(Eend::Entities::boards().clone(other.m_bodyId)), m_position(other.m_position),
      m_speed(other.m_speed), m_knockback(other.m_knockback), m_anim_time(other.m_anim_time),
      m_health(other.m_health), m_dead_time
      (other.m_dead_time
      ), m_delete(other.m_delete) {}

Dog& Dog::operator=(Dog&& other) noexcept {
    // Self-assignment detection
    if (&other == this) return *this;

    Eend::Entities::boards().erase(m_bodyId);

    // transfer ownership
    m_bodyId = Eend::Entities::boards().clone(other.m_bodyId);
    m_position = other.m_position;
    m_speed = other.m_speed;
    m_knockback = other.m_knockback;
    m_anim_time = other.m_anim_time;
    m_health = other.m_health;
    m_dead_time
     = other.m_dead_time
    ;
    m_delete = other.m_delete;

    return *this;
}
*/

void Dog::setSpeed(float speed) { Dog::speed = speed; }

Eend::Point2D Dog::getPosition() { return Dog::position; }

Eend::Point Dog::getPosition3d() {
    return Eend::Point(position.x, position.y, Park::get().elevationAtPoint(position));
}

unsigned int Dog::getDamage() { return DAMAGE; }

bool Dog::giveDamage(unsigned int damage) {
    if (damage >= health) {
        health = 0;
        Eendgine::Board* board = Eend::Entities::boards().getRef(body_id);
        board->setStrip("dead");
        board->setStripIdx(0);
        knockback *= KNOCKBACK_DEAD_MULTIPLIER;
        return true;
    } else {
        health -= damage;
        return false;
    }
}

unsigned int Dog::getHealth() { return Dog::health; }

bool Dog::shouldDelete() { return Dog::to_delete; }

void Dog::kick(Eend::Vector kick) {

    const float kick_length = glm::length(kick);

    if (kick_length == 0.0f) return;

    const float kickFactor = ((KNOCKBACK_MAX - KNOCKBACK_MIN) * kick_length) + (KNOCKBACK_MIN);

    knockback = glm::normalize(kick) * kickFactor;
}

void Dog::update() {

    Eend::Board* board_ref = Eend::Entities::boards().getRef(body_id);
    float dt = Eend::FrameLimiter::get().deltaTime;

    anim_time += dt;
    if (health == 0) {
        dead_time += dt;
    }
    if (health > 0) {
        if (anim_time > ANIM_INCREMENT_TIME) {
            anim_time = 0;
            board_ref->nextStripIdx();
        }
    } else {
        if ((dead_time > BLINK_AFTER_DEATH_TIME) && (anim_time > ANIM_INCREMENT_TIME)) {
            anim_time = 0;
            board_ref->nextStripIdx();
        }
    }

    position += knockback * dt;

    Eend::Point2D approachPoint = Trey::get().getPosition();

    if (health != 0) {
        const glm::vec2 difference = approachPoint - position;
        board_ref->setStripFlip(difference.x < 0.0f);
        if (glm::length(difference) > STOP_DISTANCE) {
            position += glm::normalize(approachPoint - position) * SPEED * dt;
        }
    }

    knockback = knockback / (1 + (dt * KNOCKBACK_DECAY_FACTOR));

    if (health == 0 && (glm::length(knockback) < KNOCKBACK_STOP)) {
        knockback = Eend::Vector2D(0.0f);
    }

    if (dead_time > DELETE_AFTER_DEATH_TIME) {
        to_delete = true;
    }

    // add some offsets for the dog visually here
    board_ref->setPosition(
        Eend::Point(position.x, position.y, Park::get().elevationAtPoint(position) + UP_OFFSET));
}

Dog::Type Dog::stringToType(std::string string) {
    // case insensitive
    transform(string.begin(), string.end(), string.begin(), ::tolower);
    if (string == "classic") return Dog::Type::Classic;
    if (string == "snow") return Dog::Type::Snow;

    return Type::Classic;
}

std::filesystem::path Dog::getDogPath(Type type) {
    std::filesystem::path dog_path;
    switch (type) {
    case Dog::Type::Classic:
        dog_path = "dog";
        break;
    case Dog::Type::Snow:
        dog_path = "dog2";
        break;
    default:
        dog_path = "dog";
    }
    return "resources" / dog_path;
}
