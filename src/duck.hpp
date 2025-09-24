#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <GL/gl.h>

#include "healthBar.hpp"
#include "terrain.hpp"

namespace Eend = Eendgine;

class Duck {
    public:
        Duck();
        ~Duck();

        void setPosition(Eend::Point position);
        void setRotation(float x, float y, float z);

        Eend::Point getPosition();
        Eend::Point2D getPosition2D();
        float getRadius();

        std::optional<Eend::Sphere> isKicking();

        void update(float dt, Terrain* terrain);

        HealthBar health;

    private:
        static constexpr float M_MOVE_SPEED = 25.0f;
        static constexpr float M_KICK_RADIUS = 10.0f;
        static constexpr float M_KICK_OFFSET = 5.0f;
        static constexpr float M_DUCK_RADIUS = 5.0f;
        static constexpr float M_GRAVITY = -1.0f; // only ducks believe in gravity

        enum Direction { UP, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN, DOWN_LEFT, LEFT, UP_LEFT };
        std::optional<Direction> getDirection();

        void handleMovement(float dt, std::optional<Duck::Direction> direction);
        void handleDirection(
            float dt, std::optional<Duck::Direction> direction, Eend::Point& position,
            float& rotation);
        void handleCollision(Terrain* terrain, Eend::Point oldPosition, Eend::Point& newPosition);

        Eend::StatueId m_bodyId;
        Eend::BoardId m_headId;
        Eend::Point m_position;
        float m_rotX;
        float m_rotY;

        bool m_kicking;
        bool m_inAir;
        float m_upVelocity;
        float m_height;
        Direction m_direction;
};
