#include <Eendgine/inputManager.hpp>

#include <cmath>
#include <optional>

#include "collision.hpp"
#include "duck.hpp"
#include "particles.hpp"

// have static last direction in update function
// use this to get new diruction if there is one, and know that keys are actually being pressed

static Particles::Behavior jumpParticleMovement =
    [](int seed, std::chrono::milliseconds time) -> std::optional<Particles::Properties> {
    if (time.count() < 1000) {
        const unsigned int bits = 8;
        const unsigned int max = std::pow(2, bits);

        const float speedScale = 0.3f;

        float x = static_cast<float>(time.count()) / static_cast<float>((seed >> bits * 0) % max) *
                  (((seed >> bits * 0) % 2) ? 1 : -1) * speedScale;
        float y = static_cast<float>(time.count()) / static_cast<float>((seed >> bits * 1) % max) *
                  (((seed >> bits * 1) % 2) ? 1 : -1) * speedScale;
        float z = static_cast<float>(time.count()) /
                  (8.0f * static_cast<float>((seed >> bits * 2) % max)) * speedScale;

        float scale = (3000.0f - static_cast<float>(time.count() * 3)) / 3000.0f;

        return std::make_optional(
            Particles::Properties(Eend::Point(x, y, z), Eend::Scale2D(scale * 2.0f)));
    }
    return std::nullopt;
};

static Particles::Behavior kickParticleMovement =
    [](int seed, std::chrono::milliseconds time) -> std::optional<Particles::Properties> {
    if (time.count() < 500) {
        return std::make_optional(Particles::Properties(Eend::Point(0.0f), Eend::Scale2D(5.0f)));
    }
    return std::nullopt;
};

Duck::Duck()
    : m_bodyId(Eend::Entities::statues().insert(std::filesystem::path("duck/statues/body"))),
      m_headId(Eend::Entities::boards().insert(std::filesystem::path("duck/boards/head"))),
      m_position(Eend::Point(0.0f)), m_rotX(0.0f), m_rotY(0.0f), m_kicking(true), m_inAir(false),
      m_upVelocity(0.0f), m_height(0.0f), m_direction(UP) {
    Eend::Entities::boards().getRef(m_headId)->setScale(Eend::Scale2D(3.5f, 3.5f));
}

Duck::~Duck() {
    Eend::Entities::statues().erase(m_bodyId);
    Eend::Entities::boards().erase(m_headId);
}

void Duck::setPosition(Eend::Point position) {
    if (position.y < m_position.y) {
        Eend::Entities::boards().getRef(m_headId)->setStrip("eyesOpen");
    } else if (position.y > m_position.y) {
        Eend::Entities::boards().getRef(m_headId)->setStrip("eyesClose");
    }
    m_position = position;
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    Eend::Board* headRef = Eend::Entities::boards().getRef(m_headId);
    bodyRef->setPosition(Eend::Point(position.x - 0.5f, position.y, position.z + 0.08f));
    headRef->setPosition(Eend::Point(position.x, position.y, position.z + 3.00f));
}

void Duck::setRotation(float x, float y, float z) {
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    bodyRef->setRotation(x, y, z);
};

Eend::Point Duck::getPosition() { return m_position; };
Eend::Point2D Duck::getPosition2D() { return Eend::Point2D(m_position.x, m_position.y); };
float Duck::getRadius() { return M_DUCK_RADIUS; }

void Duck::update(float dt, Terrain* terrain) {

    Eend::Point duckPosition = getPosition();
    Eend::Point oldDuckPosition = duckPosition;

    static float duckRotation = 0.0f;

    std::optional<Direction> currentDirection = getDirection();
    m_direction = currentDirection ? *currentDirection : m_direction;

    handleDirection(dt, currentDirection, duckPosition, duckRotation);
    handleCollision(terrain, oldDuckPosition, duckPosition);

    float heightAtPoint = terrain->heightAtPoint(Eend::Point2D(duckPosition.x, duckPosition.y));

    m_kicking = false;
    if (Eend::InputManager::get().getSpacePress() && !m_inAir) {
        m_kicking = true;
        Particles::get().create(
            duckPosition, 5, std::filesystem::path("duck/boards/poo"), jumpParticleMovement);
        m_inAir = true;
        m_upVelocity = -M_GRAVITY * 20.0f;
        m_height = heightAtPoint + 0.1f;
    } else if (m_inAir) {
        m_upVelocity += M_GRAVITY;
        m_height += (m_upVelocity * dt);
        if (m_height < heightAtPoint) {
            m_inAir = false;
            m_height = heightAtPoint;
        }
    } else {
        m_height = heightAtPoint;
    }

    duckPosition.z = m_height;

    setPosition(duckPosition);
    setRotation(0.0f, 0.0f, duckRotation);
}

bool Duck::isKicking() { return m_kicking; }

std::optional<Duck::Direction> Duck::getDirection() {
    const bool upPress = Eend::InputManager::get().getUpPress();
    const bool rightPress = Eend::InputManager::get().getRightPress();
    const bool downPress = Eend::InputManager::get().getDownPress();
    const bool leftPress = Eend::InputManager::get().getLeftPress();

    if (upPress) {
        if (rightPress) {
            return UP_RIGHT;
        }
        if (leftPress) {
            return UP_LEFT;
        }
        return UP;
    }
    if (downPress) {
        if (rightPress) {
            return DOWN_RIGHT;
        }
        if (leftPress) {
            return DOWN_LEFT;
        }
        return DOWN;
    }
    if (rightPress) {
        return RIGHT;
    } else if (leftPress) {
        return LEFT;
    }
    return std::nullopt;
}

void Duck::handleDirection(
    float dt, std::optional<Duck::Direction> direction, Eend::Point& position, float& rotation) {

    if (direction) {
        switch (*direction) {
        case UP:
            position.y += M_MOVE_SPEED * dt;
            rotation = 180;
            break;
        case UP_RIGHT:
            position.y += M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            position.x += M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            rotation = 225;
            break;
        case RIGHT:
            position.x += M_MOVE_SPEED * dt;
            rotation = 270;
            break;
        case DOWN_RIGHT:
            position.y -= M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            position.x += M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            rotation = 315;
            break;
        case DOWN:
            position.y -= M_MOVE_SPEED * dt;
            rotation = 0;
            break;
        case DOWN_LEFT:
            position.y -= M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            position.x -= M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            rotation = 45;
            break;
        case LEFT:
            position.x -= M_MOVE_SPEED * dt;
            rotation = 90;
            break;
        case UP_LEFT:
            position.y += M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            position.x -= M_MOVE_SPEED * (1 / (std::sqrt(2))) * dt;
            rotation = 135;
            break;
        }
    } else {
        rotation += 100.0f * dt;
    }
}

void Duck::handleCollision(Terrain* terrain, Eend::Point oldPosition, Eend::Point& newPosition) {
    if (!terrain->colliding(Eend::Point2D(newPosition.x, newPosition.y))) {
    } else if (!terrain->colliding(Eend::Point2D(oldPosition.x, newPosition.y))) {
        newPosition.x = oldPosition.x;
    } else if (!terrain->colliding(Eend::Point2D(newPosition.x, oldPosition.y))) {
        newPosition.y = oldPosition.y;
    } else {
        newPosition.x = oldPosition.x;
        newPosition.y = oldPosition.y;
    }
}

float Duck::getAngle() {
    switch (m_direction) {
    case UP:
        return 45.0f * 0.0f;
    case UP_RIGHT:
        return 45.0f * 1.0f;
    case RIGHT:
        return 45.0f * 2.0f;
    case DOWN_RIGHT:
        return 45.0f * 3.0f;
    case DOWN:
        return 45.0f * 4.0f;
    case DOWN_LEFT:
        return 45.0f * 5.0f;
    case LEFT:
        return 45.0f * 6.0f;
    case UP_LEFT:
        return 45.0f * 7.0f;
    }
}

void Duck::kick(Dog& dog) {
    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(), Eend::Sphere(getPosition(), M_KICK_RADIUS), getAngle(), M_KICK_SPREAD);
    if (kick) {
        dog.kick(*kick);
        dog.giveDamage(1);
    }
}
