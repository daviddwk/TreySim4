#include <Eendgine/inputManager.hpp>

#include <optional>

#include "collision.hpp"

#include "duck.hpp"
#include "duckParticles.hpp"
// have static last direction in update function
// use this to get new diruction if there is one, and know that keys are actually being pressed
//
//

Duck::Duck()
    : m_bodyId(Eend::Entities::statues().insert(std::filesystem::path("duck/statues/body"))),
      m_headId(Eend::Entities::boards().insert(std::filesystem::path("duck/boards/head"))),
      m_position(Eend::Point(0.0f)), m_rotation(Eend::Angle(0.0f)), m_kicking(true), m_inAir(false),
      m_upVelocity(0.0f), m_height(0.0f), m_direction(UP) {
    Eend::Board* head = Eend::Entities::boards().getRef(m_headId);
    head->setScale(Eend::Scale2D(3.5f, 3.5f));
    head->setStrip("eyesOpen");
}

Duck::~Duck() {
    Eend::Entities::statues().erase(m_bodyId);
    Eend::Entities::boards().erase(m_headId);
}

void Duck::setPosition(Eend::Point position) { m_position = position; }

Eend::Point Duck::getPosition() { return m_position; };
Eend::Point2D Duck::getPosition2D() { return Eend::Point2D(m_position.x, m_position.y); };
float Duck::getRadius() { return M_DUCK_RADIUS; }

void Duck::update(float dt, Terrain* terrain) {

    Eend::Point oldDuckPosition = getPosition();

    std::optional<Direction> currentDirection = getDirection();
    if (currentDirection) {
        m_direction = *currentDirection;
        m_rotation = getAngle();
        updatePosition(dt);
    } else {
        m_rotation = m_rotation + Eend::Angle(100 * dt);
    }
    handleCollision(terrain, oldDuckPosition);

    float heightAtPoint = terrain->heightAtPoint(Eend::Point2D(m_position.x, m_position.y));

    m_kicking = false;
    if (Eend::InputManager::get().getSpacePress() && !m_inAir) {
        m_kicking = true;
        Eend::Particles::get().create(m_position, 2, getKickParticleProperties(m_direction));
        Eend::Particles::get().create(m_position, 5, getJumpParticleProperties());
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

    m_position.z = m_height;

    // update entities
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    Eend::Board* headRef = Eend::Entities::boards().getRef(m_headId);

    if (m_position.y < oldDuckPosition.y) {
        headRef->setStrip("eyesOpen");
    } else if (m_position.y > oldDuckPosition.y) {
        headRef->setStrip("eyesClose");
    }

    bodyRef->setPosition(Eend::Point(m_position.x - 0.5f, m_position.y, m_position.z + 0.08f));
    headRef->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 3.00f));
    bodyRef->setRotation(0.0f, 0.0f, m_rotation.getDegrees() + 180.0f);
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

void Duck::updatePosition(float dt) {

    switch (m_direction) {
    case UP:
        m_position.y += M_MOVE_SPEED * dt;
        break;
    case UP_RIGHT:
        m_position.y += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case RIGHT:
        m_position.x += M_MOVE_SPEED * dt;
        break;
    case DOWN_RIGHT:
        m_position.y -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case DOWN:
        m_position.y -= M_MOVE_SPEED * dt;
        break;
    case DOWN_LEFT:
        m_position.y -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case LEFT:
        m_position.x -= M_MOVE_SPEED * dt;
        break;
    case UP_LEFT:
        m_position.y += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    }
}

void Duck::handleCollision(Terrain* terrain, Eend::Point& oldPosition) {
    if (!terrain->colliding(Eend::Point2D(m_position.x, m_position.y))) {
    } else if (!terrain->colliding(Eend::Point2D(oldPosition.x, m_position.y))) {
        m_position.x = oldPosition.x;
    } else if (!terrain->colliding(Eend::Point2D(m_position.x, oldPosition.y))) {
        m_position.y = oldPosition.y;
    } else {
        m_position.x = oldPosition.x;
        m_position.y = oldPosition.y;
    }
}

Eend::Angle Duck::getAngle() {
    switch (m_direction) {
    case UP:
        return Eend::Angle(45.0f) * 0.0f;
    case UP_RIGHT:
        return Eend::Angle(45.0f) * 1.0f;
    case RIGHT:
        return Eend::Angle(45.0f) * 2.0f;
    case DOWN_RIGHT:
        return Eend::Angle(45.0f) * 3.0f;
    case DOWN:
        return Eend::Angle(45.0f) * 4.0f;
    case DOWN_LEFT:
        return Eend::Angle(45.0f) * 5.0f;
    case LEFT:
        return Eend::Angle(45.0f) * 6.0f;
    case UP_LEFT:
        return Eend::Angle(45.0f) * 7.0f;
    }
}

void Duck::kick(Dog& dog) {
    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(getPosition(), M_KICK_RADIUS),
        m_rotation,
        M_KICK_SPREAD);
    if (kick) {
        dog.kick(*kick);
        dog.giveDamage(1);
    }
}
