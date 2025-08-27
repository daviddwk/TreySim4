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
        static constexpr float M_KICK_RADIUS = 15.0f;
        static constexpr float M_DUCK_RADIUS = 5.0f;
        static constexpr float M_GRAVITY = -1.0f; // only ducks believe in gravity

        Eend::StatueId m_bodyId;
        Eend::BoardId m_headId;
        Eend::Point m_position;
        float m_rotX;
        float m_rotY;

        bool m_kicking;
        bool m_inAir;
        float m_upVelocity;
        float m_height;
};
