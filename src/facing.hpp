#pragma once

#include <Eendgine/types.hpp>

enum class Direction { up, upRight, right, downRight, down, downLeft, left, upLeft };

namespace Eend = Eendgine;

class Facing {
    public:
        Facing();

        void setDirection(Direction direction);

        Direction getDirection();
        Direction getOpposite();
        Direction getUpOrDown();
        Direction getLeftOrRight();
        Eend::Angle getAngle();

    private:
        Direction m_direction;
        bool m_lastUp;
        bool m_lastRight;
};
