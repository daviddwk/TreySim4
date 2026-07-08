#include "facing.hpp"

Facing::Facing() : m_direction(Direction::down), m_lastUp(true), m_lastRight(true) {}

void Facing::setDirection(Direction direction) {
    m_direction = direction;
    switch (m_direction) {
    case Direction::up:
        m_lastUp = true;
        break;
    case Direction::upRight:
        m_lastUp = true;
        m_lastRight = true;
        break;
    case Direction::right:
        m_lastRight = true;
        break;
    case Direction::downRight:
        m_lastUp = false;
        m_lastRight = true;
        break;
    case Direction::down:
        m_lastUp = false;
        break;
    case Direction::downLeft:
        m_lastUp = false;
        m_lastRight = false;
        break;
    case Direction::left:
        m_lastRight = false;
        break;
    case Direction::upLeft:
        m_lastUp = true;
        m_lastRight = false;
        break;
    }
}

Direction Facing::getDirection() { return m_direction; }

Direction Facing::getOpposite() {
    switch (m_direction) {
    case Direction::up:
        return Direction::down;
    case Direction::upRight:
        return Direction::downLeft;
    case Direction::right:
        return Direction::left;
    case Direction::downRight:
        return Direction::upLeft;
    case Direction::down:
        return Direction::up;
    case Direction::downLeft:
        return Direction::upRight;
    case Direction::left:
        return Direction::right;
    case Direction::upLeft:
        return Direction::downRight;
    }
}

Direction Facing::getUpOrDown() { return m_lastUp ? Direction::up : Direction::down; }
Direction Facing::getLeftOrRight() { return m_lastRight ? Direction::right : Direction::left; }

Eend::Angle Facing::getAngle() {
    switch (m_direction) {
    case Direction::up:
        return Eend::Angle(45.0f) * 0.0f;
    case Direction::upRight:
        return Eend::Angle(45.0f) * 1.0f;
    case Direction::right:
        return Eend::Angle(45.0f) * 2.0f;
    case Direction::downRight:
        return Eend::Angle(45.0f) * 3.0f;
    case Direction::down:
        return Eend::Angle(45.0f) * 4.0f;
    case Direction::downLeft:
        return Eend::Angle(45.0f) * 5.0f;
    case Direction::left:
        return Eend::Angle(45.0f) * 6.0f;
    case Direction::upLeft:
        return Eend::Angle(45.0f) * 7.0f;
    }
}
