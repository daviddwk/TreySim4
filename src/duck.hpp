#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include "healthBar.hpp"

namespace Eend = Eendgine;

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

        HealthBar health;

    private:
        Eend::StatueId _bodyId;
        Eend::BoardId _headId;
        Eend::Point _position;
        float _rotX;
        float _rotY;
};
