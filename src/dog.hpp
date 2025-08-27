#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include "terrain.hpp"

namespace Eend = Eendgine;

class Dog {
    public:
        Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Terrain* terrain);
        ~Dog();

        void setSpeed(float speed);
        Eend::Point2D getPosition();
        Eend::Point getPosition3d();

        unsigned int getDamage();

        void kick(Eend::Point kick);

        void update(float dt, Eend::Point2D approachPoint);

    private:
        static constexpr float M_SPEED = 20.0f;
        static constexpr unsigned int M_DAMAGE = 2;
        static constexpr float M_ANIM_INCREMENT_TIME = 0.25f;
        static constexpr float M_KNOCKBACK_MAX = M_SPEED * 5.0f;
        static constexpr float M_KNOCKBACK_MIN = M_SPEED;
        static constexpr float M_KNOCKBACK_DECAY_FACTOR = 1.0f;
        static constexpr float M_UP_OFFSET = 2.0f;

        Eend::BoardId m_bodyId;
        Eend::Point2D m_position;
        float m_speed;
        Eend::Point2D m_knockback;
        Terrain* m_terrain;
        float m_time;
};
