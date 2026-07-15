#include "items.hpp"
#include "park.hpp"

#include <Eendgine/fatalError.hpp>

constexpr float HEIGHT_OFFSET = 5.0f;
constexpr Eend::Scale2D SCALE = Eend::Scale2D(5.0f);

Item::Item(Item::Type type, Eend::Point position)
    : m_type(type), m_position(position),
      m_sprite(Eend::Entities::boards().insert(Item::spritePath(type))) {
    Eend::Entities::boards().getRef(m_sprite)->setScale(Eend::Scale2D(SCALE));
}

Item::~Item() { Eend::Entities::boards().erase(m_sprite); }

Item::Item(Item&& other) noexcept
    : m_bodyId(/* copy entity using batch syfstem */), m_type(other.m_type),
      m_position(other.m_position) {}

Item& Item::operator=(Item&& other) noexcept {
    // Self-assignment detection
    if (&other == this) return *this;

    // delete entitiy
    // if (m_bodyId) Eend::Entities::boards().erase(*m_bodyId);

    // transfer ownership
    m_type = other.m_type;
    m_position = other.m_position;

    // release ownership
    // actually copy entity
    // other.m_bodyId = std::nullopt;

    return *this;
}

Eend::Point Item::getPosition() { return m_position; }

std::filesystem::path Item::spritePath(Item::Type type) {
    switch (type) {
    case Item::Type::doubleKick:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    default:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    }
}

Item::Type Item::typeFromString(std::string typeString) {
    if (typeString == "doubleKick") {
        return Item::Type::doubleKick;
    } else {
        Eend::fatalError("item type " + typeString + " does not exsist");
    }
}
