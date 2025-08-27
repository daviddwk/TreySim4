#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <GL/gl.h>

#include "healthBar.hpp"
#include "terrain.hpp"

namespace Eend = Eendgine;

constexpr float GRAVITY = -1.0f;

class Duck {
        // please make a position, rotation, whatever types already and decouple dependencies or
        // whatever
    public:
        Duck();
        ~Duck();

        void setPosition(Eend::Point position);
        void setRotation(float x, float y, float z);

        Eend::Point getPosition();
        Eend::Point2D getPosition2D();

        std::optional<Eend::Sphere> isKicking();

        void update(float dt, Terrain* terrain);

        HealthBar health;

    private:
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
