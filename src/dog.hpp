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

        void update(float dt, Eend::Point2D approachPoint);

    private:
        Eend::BoardId _bodyId;
        Eend::Point2D _position;
        float _speed;
        Terrain* _terrain;
        float _time;
};
