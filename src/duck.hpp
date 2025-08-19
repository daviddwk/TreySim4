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

        void update(float dt, Terrain* terrain);

        HealthBar health;

    private:
        Eend::StatueId _bodyId;
        Eend::BoardId _headId;
        Eend::Point _position;
        float _rotX;
        float _rotY;

        bool _inAir;
        float _upVelocity;
        float _height;
};
