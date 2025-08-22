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

        void kick(Eend::Point2D knockback);

        void update(float dt, Eend::Point2D approachPoint);

    private:
        Eend::BoardId m_bodyId;
        Eend::Point2D m_position;
        float m_speed;
        Eend::Point2D m_knockback;
        Terrain* m_terrain;
        float m_time;
};
